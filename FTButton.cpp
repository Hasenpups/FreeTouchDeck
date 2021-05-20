#include "FTButton.h"

static const char *module = "FTButton";
namespace FreeTouchDeck
{
    const char *enum_to_string(ButtonTypes type)
    {
        switch (type)
        {
        case ButtonTypes::STANDARD:
            return "STANDARD";
        case ButtonTypes::LATCH:
            return "LATCH";
        case ButtonTypes::MENU:
            return "MENU";
        default:
            return "Unknown button type";
        }
    }
    BMPImage *FTButton::LatchedLogo()
    {
        BMPImage *image = NULL;
        if (_jsonLatchedLogo && cJSON_IsString(_jsonLatchedLogo))
        {
            if(strlen(cJSON_GetStringValue(_jsonLatchedLogo))>0 )
            {
                ESP_LOGV(module,"Latched Logo file name is %s",cJSON_GetStringValue(_jsonLatchedLogo));
                image = GetImage(cJSON_GetStringValue(_jsonLatchedLogo));
                if (image && !image->valid)
                {
                    ESP_LOGD(module,"Latched Logo file %s is invalid.",cJSON_GetStringValue(_jsonLatchedLogo));
                    image = NULL;
                }
            }
        }
        return image;
    }
    bool FTButton::IsLabelDraw()
    {
        BMPImage *image = NULL;
        if (_jsonLogo && cJSON_IsString(_jsonLogo))
        {
            if(strlen(cJSON_GetStringValue(_jsonLogo))>0)
            {
                image = GetImage(cJSON_GetStringValue(_jsonLogo));
            }
            else
            {
                ESP_LOGD(module,"Empty logo value");
            }
        }
        if (!image || !image->valid)
        {
            return true;
        }
        return false;
    }
    BMPImage *FTButton::Logo()
    {
        BMPImage *image = NULL;

        if (_jsonLogo && cJSON_IsString(_jsonLogo))
        {
            if(strlen(cJSON_GetStringValue(_jsonLogo))>0)
            {
                ESP_LOGV(module,"Logo file name is %s",cJSON_GetStringValue(_jsonLogo));
                image = GetImage(cJSON_GetStringValue(_jsonLogo));
            }
            else
            {
                ESP_LOGD(module, "empty logo file name");
            }

        }
        if (!image || !image->valid)
        {
            ESP_LOGD(module,"Logo file name %s was not found. Defaulting to label.bmp",cJSON_GetStringValue(_jsonLogo));
            image = GetImage("label.bmp");
        }
        return image;
    }
    bool FTButton::Latch(FTAction *action)
    {
        if (ButtonType == ButtonTypes::LATCH && action->IsLatch())
        {
            switch (action->Type)
            {
            case ActionTypes::SETLATCH:
                if (!Latched)
                {
                    Latched = true;
                    Invalidate();
                }
                break;
            case ActionTypes::CLEARLATCH:
                if (Latched)
                {
                    Latched = false;
                    Invalidate();
                }
                break;
            case ActionTypes::TOGGLELATCH:
                Latched = !Latched;
                Invalidate();
                break;
            default:
                break;
            }
        }
        else
        {
            ESP_LOGW(module, "Cannot latch button. Button %s has type %s.", Label,enum_to_string(ButtonType));
            return false;
        }
        return true;
    }
    FTButton::FTButton(uint8_t index, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char menuName[31] = {0};

        _jsonLogo = button;
        cJSON *jsonActionValue = NULL;
        ButtonType = ButtonTypes::MENU;
        snprintf(menuName, sizeof(menuName), "menu%d", index + 1);
        actions.push_back(new FTAction(ActionTypes::MENU, menuName));
    }
    FTButton::FTButton(uint8_t index, cJSON *document, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char logoName[31] = {0};
        cJSON *jsonActionValue = NULL;
        cJSON *_jsonLatch = NULL;
        snprintf(logoName, sizeof(logoName), "logo%d", index);
        _jsonLogo = cJSON_GetObjectItem(document, logoName);

        strncpy(Label, logoName, sizeof(Label));
        _jsonLatchedLogo = cJSON_GetObjectItem(button, "latchlogo");
        _jsonLatch = cJSON_GetObjectItem(button, "latch");
        ButtonType = (_jsonLatch && cJSON_IsBool(_jsonLatch) && cJSON_IsTrue(_jsonLatch)) ? ButtonTypes::LATCH : ButtonTypes::STANDARD;
        cJSON *jsonActions = cJSON_GetObjectItem(button, "actionarray");
        if (!jsonActions)
        {
            ESP_LOGE(module, "Button %s does not have any action!", Logo()->LogoName);
        }
        else
        {
            uint8_t valuePos = 0;
            ESP_LOGD(module, "Button %s has %d actions", Logo()->LogoName, cJSON_GetArraySize(jsonActions));
            cJSON_ArrayForEach(jsonActionValue, cJSON_GetObjectItem(button, "valuearray"))
            {
                cJSON *jsonAction = cJSON_GetArrayItem(jsonActions, valuePos++);
                if (!jsonAction)
                {
                    ESP_LOGE(module, "Current value does not have a matching action!");
                }
                else if (FTAction::GetType(jsonAction) != ActionTypes::NONE)
                {
                    ESP_LOGD(module, "Adding action to button %s, type %s", Logo()->LogoName, enum_to_string(FTAction::GetType(jsonAction)));
                    // only push valid actions
                    auto action = new FTAction(jsonAction, jsonActionValue);
                    if (!action)
                    {
                        ESP_LOGE(module, "Could not allocate memory for action");
                    }
                    actions.push_back(action);
                }
                else
                {
                    ESP_LOGD(module, "Ignoring action type NONE for button %s", Logo()->LogoName);
                }
            }
        }
    }

    FTButton::~FTButton()
    {
        if (Label != NULL)
            free(Label);
    }
    void FTButton::Invalidate()
    {
        NeedsDraw = true;
    }
    BMPImage *FTButton::GetActiveImage()
    {
        BMPImage *image = NULL;
        if (Latched)
        {
            image = LatchedLogo();
        }
        if (!image)
        {
            image = Logo();
        }
        return image;
    }
    void FTButton::Draw(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t margin, bool force)
    {
        bool transparent = false;

        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        BMPImage *image = GetActiveImage();
        if (!image || !image->valid)
        {
            ESP_LOGE(module, "No image found, or image invalid!");
            return;
        }
        ESP_LOGV(module, "Found image structure, bitmap is %s", image->LogoName);

        uint16_t BGColor = tft.color565(image->R, image->G, image->B);
        ESP_LOGD(module, "Drawing button at [%d,%d] size: %dx%d,  with margin %d, outline : 0x%04X, BG Color: 0x%04X, Text color: 0x%04X, Text size: %d", x, y, image->w + margin, image->h + margin, margin, Outline, BGColor, TextColor, TextSize);
        tft.setFreeFont(LABEL_FONT);
        initButton(&tft, x, y, width, height, Outline, BGColor, TextColor, (char *)(IsLabelDraw() ? Label : ""), TextSize);

        if (Latched && !LatchedLogo())
        {
            ESP_LOGD(module, "Latched without a latched logo.  Drawing round rectangle");
            tft.fillRoundRect(x-width/2, y-height/2, width, height, 4, generalconfig.latchedColour);
            transparent = true;
        }

        drawButton();
        image->Draw(x, y, transparent);
    }
    uint16_t FTButton::Width()
    {
        return LatchedLogo()?max(Logo()->w, LatchedLogo()->w):Logo()->w;
    }
    uint16_t FTButton::Height()
    {
        return LatchedLogo()?max(Logo()->h, LatchedLogo()->h):Logo()->h;
    }

    void FTButton::Press()
    {
        if (IsPressed)
        {
            ESP_LOGV(module, "Button already pressed. Ignoring");
            return;
        }
        // Beep
        HandleAudio(Sounds::BEEP);
        ESP_LOGD(module, "%s Button Press detected with %d actions", enum_to_string(ButtonType), actions.size());
        for (FTAction *action : actions)
        {
            ESP_LOGD(module, "Queuing action %s", enum_to_string(action->Type), action->toString());
            if (!QueueAction(action))
            {
                ESP_LOGW(module, "Button action %s could not be queued for execution.", action->toString());
            }
        }
        if (ButtonType == ButtonTypes::LATCH)
        {
            Latched = !Latched;
            ESP_LOGD(module, "Toggling LATCH to %s", Latched ? "ACTIVE" : "INACTIVE");
        }
        IsPressed = true;
        NeedsDraw = true;
    }
    void FTButton::Release()
    {
        if (IsPressed)
        {
            ESP_LOGD(module, "Releasing button with logo %s", Logo()->LogoName);
            IsPressed = false;
            NeedsDraw = true;
        }
    }

};
