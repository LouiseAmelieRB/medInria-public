/* medDatabaseController.cpp ---
 * 
 * Author: Julien Wintz
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Tue Mar 31 11:05:39 2009 (+0200)
 * Version: $Id$
 * Last-Updated: Fri Oct 16 15:44:02 2009 (+0200)
 *           By: Julien Wintz
 *     Update #: 111
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include <QtCore>
#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractDataReader.h>
#include <dtkCore/dtkAbstractDataWriter.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkGlobal.h>
#include <dtkCore/dtkLog.h>

#include <medSql/medDatabaseController.h>

bool medDatabaseController::createConnection(void)
{
    this->mkpath(this->dataLocation() + "/");

    this->m_database = QSqlDatabase::addDatabase("QSQLITE");
    this->m_database.setDatabaseName(this->dataLocation() + "/" + "db");

    if (!m_database.open()) {
        qDebug() << DTK_COLOR_FG_RED << "Cannot open database: Unable to establish a database connection." << DTK_NOCOLOR;
        return false;
    }

    createPatientTable();
    createStudyTable();
    createSeriesTable();
    createImageTable();

    return true;
}

bool medDatabaseController::closeConnection(void)
{
    m_database.close();

    return true;
}

bool medDatabaseController::mkpath(const QString& dirPath)
{
    QDir dir; return(dir.mkpath(dirPath));
}

bool medDatabaseController::rmpath(const QString& dirPath)
{
    QDir dir; return(dir.rmpath(dirPath));
}

QString medDatabaseController::dataLocation(void) const
{
#ifdef Q_WS_MAC
    return QString(QDesktopServices::storageLocation(QDesktopServices::DataLocation))
    .remove(QCoreApplication::applicationName())
    .append(QCoreApplication::applicationName());
#else
    //qDebug() <<  QDesktopServices::storageLocation(QDesktopServices::DataLocation);

    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}

QString medDatabaseController::configLocation(void) const
{
#ifdef Q_WS_MAC
    return(QDir::homePath() + "/Library/Preferences/" + "com" + "." + QCoreApplication::organizationName() + "." + QCoreApplication::applicationName() + "." + "plist");
#else
    return(dataLocation());
#endif
}

void medDatabaseController::import(const QString& file)
{
    QDir dir(file);
    dir.setFilter(QDir::Files | QDir::Hidden);

    QStringList fileList;
    if (dir.exists()) {
      QDirIterator directory_walker(file, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
	while (directory_walker.hasNext()) {
	    fileList << directory_walker.next();
	}
    }
    else
        fileList << file;

       

    QMap<QString, QStringList> imagesToWriteMap;
    
    
    typedef dtkAbstractDataFactory::dtkAbstractDataTypeHandler dtkAbstractDataTypeHandler;
    
    QList<dtkAbstractDataTypeHandler> readers = dtkAbstractDataFactory::instance()->readers();

    int fileCount = fileList.count();
    int fileIndex = 0;

    this->setImportProgress (0);
    
    foreach (QString file, fileList) {

        QFileInfo fileInfo( file );

        dtkAbstractData* dtkdata = 0;

        for (int i=0; i<readers.size(); i++) {            
            dtkAbstractDataReader* dataReader = dtkAbstractDataFactory::instance()->reader(readers[i].first, readers[i].second);
            if (dataReader->canRead( fileInfo.filePath() )) {
                dataReader->readInformation( fileInfo.filePath() );
                dtkdata = dataReader->data();
                delete dataReader;
                break;
            }
        }
                
        if (!dtkdata)
            continue;

	
	if (!dtkdata->hasMetaData ("PatientName"))
	  dtkdata->addMetaData ("PatientName", QStringList() << fileInfo.fileName());
	if (!dtkdata->hasMetaData ("StudyDescription"))
	  dtkdata->addMetaData ("StudyDescription", QStringList() << "EmptyStudy");
	if (!dtkdata->hasMetaData ("SeriesDescription"))
	  dtkdata->addMetaData ("SeriesDescription", QStringList() << "EmptySeries");
	
	QString patientName = dtkdata->metaDataValues(tr("PatientName"))[0];
	QString studyName   = dtkdata->metaDataValues(tr("StudyDescription"))[0];
	QString seriesName  = dtkdata->metaDataValues(tr("SeriesDescription"))[0];

	QSqlQuery query(*(medDatabaseController::instance()->database()));
	QVariant id;

	QString imageFileName = this->dataLocation() + "/" + patientName.simplified() + "/" + studyName.simplified() + "/" + seriesName.simplified() + ".mhd";

	// Check if PATIENT/STUDY/SERIES/IMAGE already exists in the database

	bool imageExists = false;
	query.prepare("SELECT id FROM patient WHERE name = :name");
	query.bindValue(":name", patientName);
	if(!query.exec())
	  qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	
	if(query.first()) {
	  id = query.value(0);

	  query.prepare("SELECT id FROM study WHERE patient = :id AND name = :name");
	  query.bindValue(":id", id);
	  query.bindValue(":name", studyName);
	  if(!query.exec())
	    qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	  
	  if(query.first()) {
	    id = query.value(0);

	    query.prepare("SELECT id FROM series WHERE study = :id AND name = :name");
	    query.bindValue(":id", id);
	    query.bindValue(":name", seriesName);
	    if(!query.exec())
	      qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	
	    if(query.first()) {
	      id = query.value(0);

	      query.prepare("SELECT id FROM image WHERE series = :id AND name = :name");
	      query.bindValue(":id", id);
	      query.bindValue(":name", fileInfo.fileName());
	      
	      if(!query.exec())
		qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	      
	      if(query.first()) {
		imageExists = true;
	      }
	    } 
	  }
	}


	if (!imageExists)
	    imagesToWriteMap[ imageFileName ] << fileInfo.filePath();

	this->setImportProgress (  (int)((double)(++fileIndex) / (double)fileCount*100.0) );
    }

    this->setImportProgress ( 100 );
    
    
    // read and write images in mhd format
    QList<dtkAbstractData*> dtkDataList;
    QList<dtkAbstractDataTypeHandler> writers = dtkAbstractDataFactory::instance()->writers();
    QMap<QString, QStringList>::const_iterator it = imagesToWriteMap.begin();
    
    int imagesCount = imagesToWriteMap.count();
    int imageIndex = 0;
    
    this->setImportProgress ( 0 );
    
    for (it; it!=imagesToWriteMap.end(); it++) {
      
      dtkAbstractData *imData = NULL;
      
      for (int i=0; i<readers.size(); i++) {
        dtkAbstractDataReader* dataReader = dtkAbstractDataFactory::instance()->reader(readers[i].first, readers[i].second);
	
        if (dataReader->canRead( it.value() )) {

	    //connect (dataReader, SIGNAL (progressed (int)), this, SLOT (setImportProgress(int)));
	  
	    if (dataReader->read( it.value() )) {
                imData = dataReader->data();

		if (imData) {
		    if (!imData->hasMetaData ("FilePaths"))
	                imData->addMetaData("FilePaths", it.value());
	    
		    if (!imData->hasMetaData ("PatientName"))
		        imData->addMetaData ("PatientName", QStringList() << QFileInfo (it.value()[0]).fileName());
		    if (!imData->hasMetaData ("StudyDescription"))
		        imData->addMetaData ("StudyDescription", QStringList() << "EmptyStudy");
		    if (!imData->hasMetaData ("SeriesDescription"))
		        imData->addMetaData ("SeriesDescription", QStringList() << "EmptySeries");
		  
		    imData->addMetaData ("FileName", it.key() );
		    delete dataReader;
		    break;
		}
	    }
	    
	    //disconnect (dataReader, SIGNAL (progressed (int)), this, SLOT (setImportProgress(int)));
            delete dataReader;
        }
      }

      
      if (!imData) {
	  qDebug() << "Could not read data: " << it.value();
	  continue;
      }



      QFileInfo fileInfo (it.key());
      if (!fileInfo.dir().exists() && !this->mkpath (fileInfo.dir().path())) {
	  qDebug() << "Cannot create directory: " << fileInfo.dir().path();
	  continue;
      }

      
      for (int i=0; i<writers.size(); i++) {
	
	dtkAbstractDataWriter *dataWriter = dtkAbstractDataFactory::instance()->writer(writers[i].first, writers[i].second);
	dataWriter->setData (imData);
	
	if (dataWriter->canWrite( it.key() )) {
	  
	  if (dataWriter->write( it.key() )) {
	      dtkDataList.push_back (imData);
	      delete dataWriter;
	      break;
	  }
	}
      }

      this->setImportProgress( (int)((double)(++imageIndex) / (double)imagesCount*100.0) );
      
    }

    this->setImportProgress ( 100 );


    // Now, populate the database

    this->setImportProgress ( 0 );
    
    for (int i=0; i<dtkDataList.count(); i++) {

        dtkAbstractData *dtkdata = dtkDataList[i];
      
      	QString patientName = dtkdata->metaDataValues(tr("PatientName"))[0].simplified();
	QString studyName   = dtkdata->metaDataValues(tr("StudyDescription"))[0].simplified();
	QString seriesName  = dtkdata->metaDataValues(tr("SeriesDescription"))[0].simplified();
	QStringList filePaths = dtkdata->metaDataValues (tr("FilePaths"));

	
	//QString patientPath;
	//QString studyPath;
	QString seriesPath = dtkdata->metaDataValues (tr("FileName"))[0];

		
	QSqlQuery query(*(medDatabaseController::instance()->database()));
	QVariant id;
	
        ////////////////////////////////////////////////////////////////// PATIENT
      
        query.prepare("SELECT id FROM patient WHERE name = :name");
	query.bindValue(":name", patientName);
	if(!query.exec())
	  qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
      
	if(query.first()) {
	  id = query.value(0);
	  //patientPath = this->dataLocation() + "/" + QString().setNum (id.toInt());
	}
	else {
	  query.prepare("INSERT INTO patient (name) VALUES (:name)");
	  query.bindValue(":name", patientName);
	  query.exec(); id = query.lastInsertId();
	  
	  //patientPath = this->dataLocation() + "/" + QString().setNum (id.toInt());
	  /*
	  if (!QDir (patientPath).exists() && !this->mkpath (patientPath))
	    qDebug() << "Cannot create directory: " << patientPath;
	  */
	}
	
	
	////////////////////////////////////////////////////////////////// STUDY

	query.prepare("SELECT id FROM study WHERE patient = :id AND name = :name");
	query.bindValue(":id", id);
	query.bindValue(":name", studyName);
	if(!query.exec())
	  qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	
	if(query.first()) {
	  id = query.value(0);
	  //studyPath = patientPath + "/" + QString().setNum (id.toInt());
	}
	else {
	  query.prepare("INSERT INTO study (patient, name) VALUES (:patient, :study)");
	  query.bindValue(":patient", id);
	  query.bindValue(":study", studyName);
	  query.exec(); id = query.lastInsertId();
	  
	  //studyPath = patientPath + "/" + QString().setNum (id.toInt());
	  /*
	  if (!QDir (studyPath).exists() && !this->mkpath (studyPath))
	    qDebug() << "Cannot create directory: " << studyPath;
	  */
	}

	
	///////////////////////////////////////////////////////////////// SERIES
	
	query.prepare("SELECT * FROM series WHERE study = :id AND name = :name");
	query.bindValue(":id", id);
	query.bindValue(":name", seriesName);
	if(!query.exec())
	  qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	
	if(query.first()) {
	  id = query.value(0);
	  QVariant seCount = query.value (2);
	  QVariant seName  = query.value (3);
	  QVariant sePath  = query.value (4);
	  
	  //seriesPath = studyPath + "/" + QString().setNum (id.toInt()) + ".mhd";
	}
	else {
	  query.prepare("INSERT INTO series (study, size, name) VALUES (:study, :size, :name)");
	  query.bindValue(":study", id);
	  query.bindValue(":size", 1);
	  query.bindValue(":name", seriesName);
	  query.exec(); id = query.lastInsertId();
	  
	  //seriesPath = studyPath + "/" + QString().setNum (id.toInt()) + ".mhd";
	}
	
	
	///////////////////////////////////////////////////////////////// IMAGE

	QList<QImage> &thumbnails = dtkdata->thumbnails();
	QFileInfo seriesInfo( seriesPath );
	QString thumb_dir = seriesInfo.dir().path() + "/" + seriesName.simplified() + "/";
	if (thumbnails.count()) {	    
	    if (!this->mkpath (thumb_dir))
	        qDebug() << "Cannot create directory: " << thumb_dir;
	}
		
	for (int j=0; j<filePaths.count(); j++) {
	  
	    QFileInfo fileInfo( filePaths[j] );
	  
	    query.prepare("SELECT id FROM image WHERE series = :id AND name = :name");
	    query.bindValue(":id", id);
	    query.bindValue(":name", fileInfo.fileName());
	    
	    if(!query.exec())
	      qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	    
	    if(query.first()) {
	      ; //qDebug() << "Image" << file << "already in database";
	    }
	    else {

	        QString thumb_name = thumb_dir + QString().setNum (j) + ".jpg";
		if (j<thumbnails.count()) 
		    thumbnails[j].save(thumb_name, "JPG");
		
	        query.prepare("INSERT INTO image (series, size, name, path, instance_path, thumbnail_path) VALUES (:series, :size, :name, :path, :instance_path, :thumbnail_path)");
		query.bindValue(":series", id);
		query.bindValue(":size", 64);
		query.bindValue(":name", fileInfo.fileName());
		query.bindValue(":path", fileInfo.filePath());
		query.bindValue(":instance_path", seriesPath);
		query.bindValue(":thumbnail_path", thumb_name);
		
		if(!query.exec())
		  qDebug() << DTK_COLOR_FG_RED << query.lastError() << DTK_NOCOLOR;
	    }
	}

	this->setImportProgress( (int)((double)(i) / (double)dtkDataList.count() * 100.0) );
	
        delete dtkdata;
    }

    this->setImportProgress ( 100 );
        

    emit updated();
}

void medDatabaseController::setImportProgress (int value)
{
    emit importCompleted (value);
}

void medDatabaseController::createPatientTable(void)
{
    QSqlQuery query(*(this->database()));
    query.exec(
	"CREATE TABLE patient ("
	" id       INTEGER PRIMARY KEY,"
	" name        TEXT"
	");"
    );
}

void medDatabaseController::createStudyTable(void)
{
    QSqlQuery query(*(this->database()));

    query.exec(
	"CREATE TABLE study ("
	" id        INTEGER      PRIMARY KEY,"
        " patient   INTEGER," // FOREIGN KEY
	" name         TEXT"
	");"
    );
}

void medDatabaseController::createSeriesTable(void)
{
    QSqlQuery query(*(this->database()));
    query.exec(
	"CREATE TABLE series ("
	" id       INTEGER      PRIMARY KEY,"
        " study    INTEGER," // FOREIGN KEY
	" size     INTEGER,"
	" name        TEXT,"
        " path        TEXT"
	");"
    );
}

void medDatabaseController::createImageTable(void)
{
    QSqlQuery query(*(this->database()));
    query.exec(
	"CREATE TABLE image ("
	" id       INTEGER      PRIMARY KEY,"
        " series   INTEGER," // FOREIGN KEY
	" size     INTEGER,"
	" name        TEXT,"
        " path        TEXT,"
	" instance_path TEXT,"
	" thumbnail_path TEXT"
        " slice    INTEGER "	
	");"
    );
}

medDatabaseController *medDatabaseController::m_instance = NULL;
