#include <Arduino.h>
#include "T-Keyboard-S3-Pro_Drive.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "icon_16Bit.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "Arduino_DriveBus_Library.h"

#define KEY1_SET KEY_LEFT_ARROW
#define KEY2_SET KEY_UP_ARROW
#define KEY3_SET KEY_DOWN_ARROW
#define KEY4_SET KEY_RIGHT_ARROW
#define KEY5_SET KEY_RETURN

#define KEY_HID_R 'r'

#define IIC_MAIN_DEVICE_ADDRESS 0x01
#define IIC_LCD_CS_DEVICE_DELAY 20

enum KNOB_State
{
    KNOB_NULL,
    KNOB_INCREMENT,
    KNOB_DECREMENT,
};

int8_t KNOB_Data = 0;
bool KNOB_Trigger_Flag = false;
uint8_t KNOB_State_Flag = KNOB_State::KNOB_NULL;

//  0B000000[KNOB_DATA_A][KNOB_DATA_B]
uint8_t KNOB_Previous_Logical = 0B00000000;

uint8_t IIC_Master_Receive_Data;

size_t IIC_Bus_CycleTime = 0;
size_t KNOB_CycleTime = 0;

uint8_t KEY1_Lock = 0;
uint8_t KEY1_Press_Delay = 0;
uint8_t KEY2_Lock = 0;
uint8_t KEY2_Press_Delay = 0;
uint8_t KEY3_Lock = 0;
uint8_t KEY3_Press_Delay = 0;
uint8_t KEY4_Lock = 0;
uint8_t KEY4_Press_Delay = 0;
uint8_t KEY5_Lock = 0;
uint8_t KEY5_Press_Delay = 0;

std::vector<unsigned char> IIC_Device_ID_Registry_Scan;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

USBHIDKeyboard Keyboard;

/*LCD*/
bool IIC_Device_ID_State = false;
std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;
Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
    bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

void KNOB_Logical_Scan_Loop(void)
{
    uint8_t KNOB_Logical_Scan = 0B00000000;

    if (digitalRead(KNOB_DATA_A) == 1)
    {
        KNOB_Logical_Scan |= 0B00000010;
    }
    else
    {
        KNOB_Logical_Scan &= 0B11111101;
    }

    if (digitalRead(KNOB_DATA_B) == 1)
    {
        KNOB_Logical_Scan |= 0B00000001;
    }
    else
    {
        KNOB_Logical_Scan &= 0B11111110;
    }

    if (KNOB_Previous_Logical != KNOB_Logical_Scan)
    {
        if (KNOB_Logical_Scan == 0B00000000 || KNOB_Logical_Scan == 0B00000011)
        {
            KNOB_Previous_Logical = KNOB_Logical_Scan;
            KNOB_Trigger_Flag = true;
        }
        else
        {
            if (KNOB_Logical_Scan == 0B00000010)
            {
                switch (KNOB_Previous_Logical)
                {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;

                default:
                    break;
                }
            }
            if (KNOB_Logical_Scan == 0B00000001)
            {
                switch (KNOB_Previous_Logical)
                {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;

                default:
                    break;
                }
            }
        }
        // delay(10);
    }
}

void IIC_KEY_Read_Loop(void)
{
    if (IIC_Device_ID_Registry_Scan.size() > 0)
    {
        for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++)
        {
            IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Registry_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                                     &IIC_Master_Receive_Data, 1);

            T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
            key_trigger_temp.ID = IIC_Device_ID_Registry_Scan[i];
            key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

            KEY_Trigger.push_back(key_trigger_temp);
        }
    }
}

void IIC_KEY_Trigger_Loop(void)
{
    if (KEY_Trigger.size() > 0)
    {
        switch (KEY_Trigger[0].ID)
        {
        case 0x01:
            if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
            {
                if (KEY1_Lock == 0)
                {
                    Keyboard.press(KEY1_SET);
                }

                if (KEY1_Press_Delay >= 100)
                {
                    KEY1_Press_Delay = 100;
                    Keyboard.press(KEY1_SET);
                }
                KEY1_Press_Delay++;
                KEY1_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY1_SET);
                KEY1_Press_Delay = 0;
                KEY1_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
            {
                if (KEY2_Lock == 0)
                {
                    Keyboard.press(KEY2_SET);
                }

                if (KEY2_Press_Delay >= 100)
                {
                    KEY2_Press_Delay = 100;
                    Keyboard.press(KEY2_SET);
                }
                KEY2_Press_Delay++;
                KEY2_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY2_SET);
                KEY2_Press_Delay = 0;
                KEY2_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
            {
                if (KEY3_Lock == 0)
                {
                    Keyboard.press(KEY3_SET);
                }

                if (KEY3_Press_Delay >= 100)
                {
                    KEY3_Press_Delay = 100;
                    Keyboard.press(KEY3_SET);
                }
                KEY3_Press_Delay++;
                KEY3_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY3_SET);
                KEY3_Press_Delay = 0;
                KEY3_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
            {
                if (KEY4_Lock == 0)
                {
                    Keyboard.press(KEY4_SET);
                }

                if (KEY4_Press_Delay >= 100)
                {
                    KEY4_Press_Delay = 100;
                    Keyboard.press(KEY4_SET);
                }
                KEY4_Press_Delay++;
                KEY4_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY4_SET);
                KEY4_Press_Delay = 0;
                KEY4_Lock = 0;
            }

            if ((KEY_Trigger[0].Trigger_Data & 0B00000001) == 1)
            {
                if (KEY5_Lock == 0)
                {
                    Keyboard.press(KEY5_SET);
                }

                if (KEY5_Press_Delay >= 100)
                {
                    KEY5_Press_Delay = 100;
                    Keyboard.press(KEY5_SET);
                }
                KEY5_Press_Delay++;
                KEY5_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY5_SET);
                KEY5_Press_Delay = 0;
                KEY5_Lock = 0;
            }
            break;

        default:
            Keyboard.release(KEY1_SET);
            Keyboard.release(KEY2_SET);
            Keyboard.release(KEY3_SET);

            Keyboard.release(KEY4_SET
            
            );
            Keyboard.release(KEY5_SET);
            break;
        }
        KEY_Trigger.erase(KEY_Trigger.begin());
    }
}

void KNOB_Trigger_Loop(void)
{
    KNOB_Logical_Scan_Loop();

    if (KNOB_Trigger_Flag == true)
    {
        KNOB_Trigger_Flag = false;

        switch (KNOB_State_Flag)
        {
        case KNOB_State::KNOB_INCREMENT:
            KNOB_Data++;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            Keyboard.press(KEY2_SET);
            delay(20);
            Keyboard.release(KEY2_SET);
            delay(50);

            break;
        case KNOB_State::KNOB_DECREMENT:
            KNOB_Data--;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            Keyboard.press(KEY3_SET);
            delay(20);
            Keyboard.release(KEY3_SET);
            delay(50);

            break;

        default:
            break;
        }
    }
}

void Task1(void *pvParameters)
{
    while (1)
    {
        IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Registry_Scan);
        if (millis() > IIC_Bus_CycleTime)
        {
            if (IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Scan) == true)
            {
                if (IIC_Device_ID_Scan.size() != IIC_Device_ID_Registry.size())
                {
                    IIC_Device_ID_State = true;
                }
            }

            IIC_Bus_CycleTime = millis() + 10; // 10ms
        }
        delay(1);
    }
}

void Select_Screen_All(std::vector<unsigned char> device_id, bool select)
{
    if (select == true)
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111);
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    }
    else
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000);
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    }
}

void Iocn_Show(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);

    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000);
        delay(IIC_LCD_CS_DEVICE_DELAY);
    } 
    gfx->setCursor(50,46);
    gfx->setTextSize(5);
    gfx->printf("<");

    delay(50);
    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00001000);
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
    //gfx->draw16bitRGBBitmap(0, 0, ICON_KEY2, 128, 128);
    gfx->setCursor(39,46);
    gfx->setTextSize(5);
    gfx->printf("/\\");

    delay(50);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000100);
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
    //gfx->draw16bitRGBBitmap(0, 0, ICON_KEY3, 128, 128);
    gfx->setCursor(39,46);
    gfx->setTextSize(5);
    gfx->printf("\\/");

    delay(50);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000010);
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
    //gfx->draw16bitRGBBitmap(0, 0, ICON_KEY4, 128, 128);
    gfx->setCursor(50,46); 
    gfx->setTextSize(5);
    gfx->printf(">");

    delay(50);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000001);
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_note, 128, 128);

    Select_Screen_All(device_id, false);
}

void LCD_Initialization(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);

    gfx->begin(4500000);
    gfx->fillScreen(BLACK);
    delay(500);

    Select_Screen_All(device_id, false);
}

void Print_IIC_Info(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);

    gfx->fillScreen(WHITE);
    gfx->setCursor(15, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->printf("IIC Info");

    Select_Screen_All(device_id, false);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(KNOB_DATA_A, INPUT_PULLUP);
    pinMode(KNOB_DATA_B, INPUT_PULLUP);

    while (IIC_Bus->begin() == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");

    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

    delay(100);

    Keyboard.begin();
    USB.begin();

    /*LCD*/
    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    digitalWrite(LCD_RST, LOW);
    delay(100);
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    ledcAttachPin(LCD_BL, 1);
    ledcSetup(1, 2000, 8);
    ledcWrite(1, 0); // brightness 0 - 255

    while (1)
    {
        bool temp = false;
        if (IIC_Device_ID_State == true)
        {
            delay(100);

            Serial.printf("Find IIC ID: %#X\n", IIC_Device_ID_Scan[0]);

            if (IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS)
            {
                IIC_Device_ID_Registry.push_back(IIC_Device_ID_Scan[0]);

                std::vector<unsigned char> vector_temp;
                vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

                LCD_Initialization(vector_temp);

                temp = true;
            }
        }
        else
        {
            temp = false;
        }

        if (temp == true)
        {
            Serial.println("IIC_Bus select LCD_CS successful");
            break;
        }
        else
        {
            Serial.println("IIC ID not found");
            delay(200);
        }
    }

    if (IIC_Bus->IIC_ReadC8_Delay_Data(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION, 20, &IIC_Master_Receive_Data, 1) == true)
    {
        Serial.printf("STM32 dirve firmware version: %#X \n", IIC_Master_Receive_Data);
    }

    std::vector<unsigned char> vector_temp;
    vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);
    Print_IIC_Info(vector_temp);
    Iocn_Show(vector_temp);
}

void loop()
{
    IIC_KEY_Read_Loop();
    IIC_KEY_Trigger_Loop();
    if (millis() > KNOB_CycleTime)
    {
        KNOB_Logical_Scan_Loop();
        KNOB_CycleTime = millis() + 20; // 20ms
    }
    KNOB_Trigger_Loop();
}
