#include "ui/UICheckBox.h"

class SimpleCheckBox : public ax::ui::AbstractCheckButton
{
public:
    static SimpleCheckBox* create(const std::string& normalImage, const std::string& selectedImage)
    {
        SimpleCheckBox* checkbox = new SimpleCheckBox();
        if (checkbox && checkbox->init(normalImage, selectedImage))
        {
            checkbox->autorelease();
            return checkbox;
        }
        delete checkbox;
        return nullptr;
    }

    void setSelected(bool selected)
    {
        if (_isSelected == selected)
            return;

        // update internal state
        _isSelected = selected;

        // call base method to keep axmol behavior
        AbstractCheckButton::setSelected(selected);

        // update texture
        updateTexture();
    }

    void addEventListener(const std::function<void(ax::Object*, ax::ui::CheckBox::EventType)>& callback)
    {
        _eventCallback = callback;
    }

protected:
    std::string _normalImage;
    std::string _selectedImage;
    std::function<void(ax::Object*, ax::ui::CheckBox::EventType)> _eventCallback;

    bool init(const std::string& normalImage, const std::string& selectedImage)
    {
        _normalImage   = normalImage;
        _selectedImage = selectedImage;

        // initialize abstractcheckbutton properly
        if (!AbstractCheckButton::init(_normalImage, "", "", "", "", ax::ui::Widget::TextureResType::LOCAL))
        {
            return false;
        }

        // remove default cross icon, as we are changing the background texture directly
        if (_frontCrossRenderer)
        {
            _frontCrossRenderer->setVisible(false);
        }

        // apply the initial texture correctly
        updateTexture();

        return true;
    }

    void onPressStateChangedToNormal() override
    {
        // toggle state when releasing the button
        setSelected(!_isSelected);
        dispatchSelectChangedEvent(_isSelected);
    }

    void dispatchSelectChangedEvent(bool selected) override
    {
        // trigger state change event using addEventListener compatibility
        if (_eventCallback)
        {
            _eventCallback(this,
                           selected ? ax::ui::CheckBox::EventType::SELECTED : ax::ui::CheckBox::EventType::UNSELECTED);
        }
    }

    void updateTexture()
    {
        if (_backGroundBoxRenderer)
        {
            // switch texture correctly between checked and unchecked
            _backGroundBoxRenderer->setTexture(_isSelected ? _selectedImage : _normalImage);
        }
    }
};
