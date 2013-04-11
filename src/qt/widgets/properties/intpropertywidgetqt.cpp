#include <inviwo/qt/widgets/properties/intpropertywidgetqt.h>

#include <QHBoxLayout>
#include <QLabel>

namespace inviwo {

IntPropertyWidgetQt::IntPropertyWidgetQt(IntProperty* property) : property_(property) {
    generateWidget();
    updateFromProperty();
}

void IntPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(new QLabel(QString::fromStdString(property_->getDisplayName())));
    sliderWidget_ = new IntSliderWidgetQt(property_->getMinValue(), property_->getMaxValue(), property_->getIncrement());
    connect(sliderWidget_->getSlider(), SIGNAL(valueChanged(int)), this, SLOT(setPropertyValueFromSlider()));
    connect(sliderWidget_->getSpinBox(), SIGNAL(valueChanged(int)),this,SLOT(setPropertyValueFromSpinBox()));
    hLayout->addWidget(sliderWidget_);
    setLayout(hLayout);
    generatesSettingsWidget();
}

void IntPropertyWidgetQt::updateFromProperty() {
    sliderWidget_->setRange(property_->getMinValue(), property_->getMaxValue());
    sliderWidget_->setValue(property_->get());
    sliderWidget_->setIncrement(property_->getIncrement());
    sliderWidget_->updateValueSpinBox();
}

void IntPropertyWidgetQt::setPropertyValueFromSpinBox() {
    sliderWidget_->updateValueSlider();
    property_->set(sliderWidget_->getValue());
}

void IntPropertyWidgetQt::setPropertyValueFromSlider() {
    sliderWidget_->updateValueSpinBox();
    property_->set(sliderWidget_->getValue());
}

void IntPropertyWidgetQt::showContextMenu( const QPoint& pos ) {
    
    QPoint globalPos = sliderWidget_->mapToGlobal(pos);

    QAction* selecteditem = settingsMenu_->exec(globalPos);
    if (selecteditem == settingsMenu_->actions().at(0)) {
        settingsWidget_->reload();
        settingsWidget_->show();
    }
    else if (selecteditem == settingsMenu_->actions().at(1)) {
        //Set current value of the slider to min value of the property
        property_->setMinValue(sliderWidget_->getValue());
        updateFromProperty();
    }
    else if (selecteditem == settingsMenu_->actions().at(2)){
        //Set current value of the slider to max value of the property
        property_->setMaxValue(sliderWidget_->getValue());
        updateFromProperty();
    }
}

void IntPropertyWidgetQt::generatesSettingsWidget() {
    settingsWidget_ = new PropertySettingsWidgetQt(property_);
    settingsMenu_ = new QMenu();
    settingsMenu_->addAction("Property settings");
    settingsMenu_->addAction("Set as Min");
    settingsMenu_->addAction("Set as Max");
    sliderWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(sliderWidget_,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(showContextMenu(const QPoint&)));
}

} // namespace
