#include "medToolBoxDiffusionTensorView.h"
#include <dtkCore/dtkAbstractViewInteractor.h>
#include <dtkCore/dtkAbstractView.h>
#include <math.h>

class medToolBoxDiffusionTensorViewPrivate
{
public:
    QComboBox*    glyphShape;
    QSlider*      sampleRate;
    QCheckBox*    flipX;
    QCheckBox*    flipY;
    QCheckBox*    flipZ;
    QRadioButton* eigenVectorV1;
    QRadioButton* eigenVectorV2;
    QRadioButton* eigenVectorV3;
    QCheckBox*    reverseBackgroundColor;
    QSlider*      glyphResolution;
    QSlider*      minorScaling;
    QSlider*      majorScaling;
    QCheckBox*    hideShowAxial;
    QCheckBox*    hideShowCoronal;
    QCheckBox*    hideShowSagittal;

    QStringList glyphShapes;
};

medToolBoxDiffusionTensorView::medToolBoxDiffusionTensorView(QWidget *parent) : medToolBox(parent), d(new medToolBoxDiffusionTensorViewPrivate)
{
    QWidget* displayWidget = new QWidget(this);

    d->glyphShapes = *(new QStringList());
    d->glyphShapes << "Lines" << "Disks" << "Arrows" << "Cubes" << "Cylinders" << "Ellipsoids" << "Superquadrics";

    // combobox to control the glyph shape
    d->glyphShape = new QComboBox(displayWidget);
    d->glyphShape->addItems(d->glyphShapes);

    QHBoxLayout* glyphShapeLayout = new QHBoxLayout;
    glyphShapeLayout->addWidget(new QLabel("Shape: "));
    glyphShapeLayout->addWidget(d->glyphShape);

    // slider to control sample rate
    d->sampleRate =  new QSlider(Qt::Horizontal, displayWidget);
    d->sampleRate->setMinimum(1);
    d->sampleRate->setMaximum(10);
    d->sampleRate->setSingleStep(1);
    d->sampleRate->setValue(1);

    QSpinBox* sampleRateSpinBox = new QSpinBox(displayWidget);
    sampleRateSpinBox->setMinimum(1);
    sampleRateSpinBox->setMaximum(10);
    sampleRateSpinBox->setSingleStep(1);
    sampleRateSpinBox->setValue(1);

    QHBoxLayout* sampleRateLayout = new QHBoxLayout;
    sampleRateLayout->addWidget(new QLabel("Sample rate: "));
    sampleRateLayout->addWidget(d->sampleRate);
    sampleRateLayout->addWidget(sampleRateSpinBox);

    connect(d->sampleRate, SIGNAL(valueChanged(int)), sampleRateSpinBox, SLOT(setValue(int)));
    connect(sampleRateSpinBox, SIGNAL(valueChanged(int)), d->sampleRate, SLOT(setValue(int)));

    // flipX, flipY and flipZ checkboxes
    d->flipX = new QCheckBox("Flip X", displayWidget);
    d->flipY = new QCheckBox("Flip Y", displayWidget);
    d->flipZ = new QCheckBox("Flip Z", displayWidget);

    QHBoxLayout* flipAxesLayout = new QHBoxLayout;
    flipAxesLayout->addWidget(d->flipX);
    flipAxesLayout->addWidget(d->flipY);
    flipAxesLayout->addWidget(d->flipZ);

    // eigen vectors
    d->eigenVectorV1 = new QRadioButton("v1", displayWidget);
    d->eigenVectorV2 = new QRadioButton("v2", displayWidget);
    d->eigenVectorV3 = new QRadioButton("v3", displayWidget);

    QButtonGroup *eigenVectorRadioGroup = new QButtonGroup(displayWidget);
    eigenVectorRadioGroup->addButton(d->eigenVectorV1);
    eigenVectorRadioGroup->addButton(d->eigenVectorV2);
    eigenVectorRadioGroup->addButton(d->eigenVectorV3);
    eigenVectorRadioGroup->setExclusive(true);

    QHBoxLayout *eigenVectorGroupLayout = new QHBoxLayout;
    eigenVectorGroupLayout->addWidget(d->eigenVectorV1);
    eigenVectorGroupLayout->addWidget(d->eigenVectorV2);
    eigenVectorGroupLayout->addWidget(d->eigenVectorV3);
    //eigenVectorGroupLayout->addStretch(1);

    QVBoxLayout* eigenLayout = new QVBoxLayout;
    eigenLayout->addWidget(new QLabel("Eigen Vector for color-coding:"));
    eigenLayout->addLayout(eigenVectorGroupLayout);

    d->eigenVectorV1->setChecked(true);

    // reverse background color
    d->reverseBackgroundColor = new QCheckBox("Reverse Background Color", displayWidget);

    // slider to control glyph resolution
    d->glyphResolution =  new QSlider(Qt::Horizontal, displayWidget);
    d->glyphResolution->setMinimum(2);
    d->glyphResolution->setMaximum(20);
    d->glyphResolution->setSingleStep(1);
    d->glyphResolution->setValue(6);

    QSpinBox* glyphResolutionSpinBox = new QSpinBox(displayWidget);
    glyphResolutionSpinBox->setMinimum(2);
    glyphResolutionSpinBox->setMaximum(20);
    glyphResolutionSpinBox->setSingleStep(1);
    glyphResolutionSpinBox->setValue(6);

    QHBoxLayout* glyphResolutionLayout = new QHBoxLayout;
    glyphResolutionLayout->addWidget(new QLabel("Glyph resolution: "));
    glyphResolutionLayout->addWidget(d->glyphResolution);
    glyphResolutionLayout->addWidget(glyphResolutionSpinBox);

    connect(d->glyphResolution, SIGNAL(valueChanged(int)), glyphResolutionSpinBox, SLOT(setValue(int)));
    connect(glyphResolutionSpinBox, SIGNAL(valueChanged(int)), d->glyphResolution, SLOT(setValue(int)));

    // minor scaling
    d->minorScaling =  new QSlider(Qt::Horizontal, displayWidget);
    d->minorScaling->setMinimum(1);
    d->minorScaling->setMaximum(9);
    d->minorScaling->setSingleStep(1);
    d->minorScaling->setValue(1);

    QSpinBox* minorScalingSpinBox = new QSpinBox(displayWidget);
    minorScalingSpinBox->setMinimum(1);
    minorScalingSpinBox->setMaximum(9);
    minorScalingSpinBox->setSingleStep(1);
    minorScalingSpinBox->setValue(1);

    QHBoxLayout* minorScalingLayout = new QHBoxLayout;
    minorScalingLayout->addWidget(new QLabel("Minor scaling: "));
    minorScalingLayout->addWidget(d->minorScaling);
    minorScalingLayout->addWidget(minorScalingSpinBox);

    connect(d->minorScaling, SIGNAL(valueChanged(int)), minorScalingSpinBox, SLOT(setValue(int)));
    connect(minorScalingSpinBox, SIGNAL(valueChanged(int)), d->minorScaling, SLOT(setValue(int)));

    // major scaling
    d->majorScaling =  new QSlider(Qt::Horizontal, displayWidget);
    d->majorScaling->setMinimum(-10);
    d->majorScaling->setMaximum(10);
    d->majorScaling->setSingleStep(1);
    d->majorScaling->setValue(0);

    QSpinBox* majorScalingSpinBox = new QSpinBox(displayWidget);
    majorScalingSpinBox->setMinimum(-10);
    majorScalingSpinBox->setMaximum(10);
    majorScalingSpinBox->setSingleStep(1);
    majorScalingSpinBox->setValue(0);

    QHBoxLayout* majorScalingLayout = new QHBoxLayout;
    majorScalingLayout->addWidget(new QLabel("Major scaling (10^n): "));
    majorScalingLayout->addWidget(d->majorScaling);
    majorScalingLayout->addWidget(majorScalingSpinBox);

    connect(d->majorScaling, SIGNAL(valueChanged(int)), majorScalingSpinBox, SLOT(setValue(int)));
    connect(majorScalingSpinBox, SIGNAL(valueChanged(int)), d->majorScaling, SLOT(setValue(int)));

    // hide or show axial, coronal, and sagittal
    d->hideShowAxial = new QCheckBox("Axial", displayWidget);
    d->hideShowCoronal = new QCheckBox("Coronal", displayWidget);
    d->hideShowSagittal = new QCheckBox("Sagittal", displayWidget);

    d->hideShowAxial->setChecked(true);
    d->hideShowCoronal->setChecked(true);
    d->hideShowSagittal->setChecked(true);

    QHBoxLayout* slicesLayout = new QHBoxLayout;
    slicesLayout->addWidget(d->hideShowAxial);
    slicesLayout->addWidget(d->hideShowCoronal);
    slicesLayout->addWidget(d->hideShowSagittal);

    // layout all the controls in the toolbox
    QVBoxLayout* layout = new QVBoxLayout(displayWidget);
    layout->addLayout(glyphShapeLayout);
    layout->addLayout(sampleRateLayout);
    //layout->addWidget(new QLabel("Flip axes:"));
    layout->addLayout(flipAxesLayout);
    layout->addLayout(eigenLayout);
    layout->addWidget(d->reverseBackgroundColor);
    layout->addLayout(glyphResolutionLayout);
    layout->addLayout(minorScalingLayout);
    layout->addLayout(majorScalingLayout);
    layout->addWidget(new QLabel("Hide or show slices:"));
    layout->addLayout(slicesLayout);


    // connect all the signals
    connect(d->glyphShape,              SIGNAL(currentIndexChanged(const QString&)), this, SIGNAL(glyphShapeChanged(const QString&)));
    connect(d->sampleRate,              SIGNAL(valueChanged(int)),                   this, SIGNAL(sampleRateChanged(int)));
    connect(d->glyphResolution,         SIGNAL(valueChanged(int)),                   this, SIGNAL(glyphResolutionChanged(int)));

    // some signals (checkboxes) require one more step to translate from Qt::CheckState to bool
    connect(d->flipX,                   SIGNAL(stateChanged(int)),                   this, SLOT(onFlipXCheckBoxStateChanged(int)));
    connect(d->flipY,                   SIGNAL(stateChanged(int)),                   this, SLOT(onFlipYCheckBoxStateChanged(int)));
    connect(d->flipZ,                   SIGNAL(stateChanged(int)),                   this, SLOT(onFlipZCheckBoxStateChanged(int)));
    connect(d->reverseBackgroundColor,  SIGNAL(stateChanged(int)),                   this, SLOT(onReverseBackgroundColorChanged(int)));
    connect(d->hideShowAxial,           SIGNAL(stateChanged(int)),                   this, SLOT(onHideShowAxialChanged(int)));
    connect(d->hideShowCoronal,         SIGNAL(stateChanged(int)),                   this, SLOT(onHideShowCoronalChanged(int)));
    connect(d->hideShowSagittal,        SIGNAL(stateChanged(int)),                   this, SLOT(onHideShowSagittalChanged(int)));

    // we also need to translate radio buttons boolean states to an eigen vector
    connect(d->eigenVectorV1,   SIGNAL(toggled(bool)),                       this, SLOT(onEigenVectorV1Toggled(bool)));
    connect(d->eigenVectorV2,   SIGNAL(toggled(bool)),                       this, SLOT(onEigenVectorV2Toggled(bool)));
    connect(d->eigenVectorV3,   SIGNAL(toggled(bool)),                       this, SLOT(onEigenVectorV3Toggled(bool)));

    // we need to calculate one single number for the scale, out of the minor and major scales
    connect(d->minorScaling,            SIGNAL(valueChanged(int)),                   this, SLOT(onMinorScalingChanged(int)));
    connect(d->majorScaling,            SIGNAL(valueChanged(int)),                   this, SLOT(onMajorScalingChanged(int)));

    this->setTitle("Tensor View");
    this->addWidget(displayWidget);
}

medToolBoxDiffusionTensorView::~medToolBoxDiffusionTensorView()
{
    delete d;
    d = NULL;
}

QString medToolBoxDiffusionTensorView::glyphShape(void)
{
    return d->glyphShape->currentText();
}

int medToolBoxDiffusionTensorView::sampleRate(void)
{
    return d->sampleRate->value();
}

bool medToolBoxDiffusionTensorView::isFlipX(void)
{
    return d->flipX->checkState() == Qt::Checked;
}

bool medToolBoxDiffusionTensorView::isFlipY(void)
{
    return d->flipY->checkState() == Qt::Checked;
}

bool medToolBoxDiffusionTensorView::isFlipZ(void)
{
    return d->flipZ->checkState() == Qt::Checked;
}

int medToolBoxDiffusionTensorView::eigenVector(void)
{
    if (d->eigenVectorV1->isChecked())
    {
        return 1;
    }
    else if (d->eigenVectorV2->isChecked())
    {
        return 2;
    }
    else if (d->eigenVectorV3->isChecked())
    {
        return 3;
    }
}

int medToolBoxDiffusionTensorView::glyphResolution(void)
{
    return d->glyphResolution->value();
}

double medToolBoxDiffusionTensorView::scale(void)
{
    int minorScale = d->minorScaling->value();
    int majorScaleExponent = d->majorScaling->value();
    double majorScale = pow(10.0, majorScaleExponent);
    double scale = majorScale * minorScale;
    return scale;
}

bool medToolBoxDiffusionTensorView::isShowAxial(void)
{
    return d->hideShowAxial->checkState() == Qt::Checked;
}

bool medToolBoxDiffusionTensorView::isShowCoronal(void)
{
    return d->hideShowCoronal->checkState() == Qt::Checked;
}

bool medToolBoxDiffusionTensorView::isShowSagittal(void)
{
    return d->hideShowSagittal->checkState() == Qt::Checked;
}

void medToolBoxDiffusionTensorView::onFlipXCheckBoxStateChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit flipX(false);
    else if (checkBoxState == Qt::Checked)
        emit flipX(true);
}

void medToolBoxDiffusionTensorView::onFlipYCheckBoxStateChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit flipY(false);
    else if (checkBoxState == Qt::Checked)
        emit flipY(true);
}

void medToolBoxDiffusionTensorView::onFlipZCheckBoxStateChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit flipZ(false);
    else if (checkBoxState == Qt::Checked)
        emit flipZ(true);
}

void medToolBoxDiffusionTensorView::onReverseBackgroundColorChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit reverseBackgroundColor(false);
    else if (checkBoxState == Qt::Checked)
        emit reverseBackgroundColor(true);
}

void medToolBoxDiffusionTensorView::onEigenVectorV1Toggled(bool isSelected)
{
    if (isSelected)
        emit eigenVectorChanged(1);
}

void medToolBoxDiffusionTensorView::onEigenVectorV2Toggled(bool isSelected)
{
    if (isSelected)
        emit eigenVectorChanged(2);
}

void medToolBoxDiffusionTensorView::onEigenVectorV3Toggled(bool isSelected)
{
    if (isSelected)
        emit eigenVectorChanged(3);
}

void medToolBoxDiffusionTensorView::onMinorScalingChanged(int minorScale)
{
    int majorScaleExponent = d->majorScaling->value();
    double majorScale = pow(10.0, majorScaleExponent);
    double scale = majorScale * minorScale;
    emit scalingChanged(scale);
}

void medToolBoxDiffusionTensorView::onMajorScalingChanged(int majorScaleExponent)
{
    int minorScale = d->minorScaling->value();
    double majorScale = pow(10.0, majorScaleExponent);
    double scale = majorScale * minorScale;
    emit scalingChanged(scale);
}

void medToolBoxDiffusionTensorView::onHideShowAxialChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit hideShowAxial(false);
    else if (checkBoxState == Qt::Checked)
        emit hideShowAxial(true);
}

void medToolBoxDiffusionTensorView::onHideShowCoronalChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit hideShowCoronal(false);
    else if (checkBoxState == Qt::Checked)
        emit hideShowCoronal(true);
}

void medToolBoxDiffusionTensorView::onHideShowSagittalChanged(int checkBoxState)
{
    if (checkBoxState == Qt::Unchecked)
        emit hideShowSagittal(false);
    else if (checkBoxState == Qt::Checked)
        emit hideShowSagittal(true);
}

void medToolBoxDiffusionTensorView::update (dtkAbstractView *view)
{
    if (!view)
        return;

    // the tensor view toolbox is expected to control all tensors
    // i.e. is general to all tensors, hence we do not update its values
    // for every view

//    //dtkAbstractViewInteractor* interactor = view->interactor("Tensor");
//    dtkAbstractViewInteractor* interactor = view->interactor("v3dViewTensorInteractor");
//
//    if(interactor)
//    {
//        QString glyphShape = interactor->property("GlyphShape");
//
//        int index = d->glyphShapes.indexOf(glyphShape);
//
//        d->glyphShape->blockSignals(true);
//        d->glyphShape->setCurrentIndex(index);
//        d->glyphShape->blockSignals(false);
//    }
}
