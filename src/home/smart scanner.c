#include <Arduino_GFX_Library.h>

#define BLACK 0x0000
#define WHITE 0xFFFF

//====================================================
// DISPLAY PINS
//====================================================

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0, true);

//====================================================
// TCS3200
//====================================================

#define S0 25
#define S1 26
#define S2 27
#define S3 14
#define SENSOR_OUT 19

//====================================================
// VARIABLES
//====================================================

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

float smoothR = 0;
float smoothG = 0;
float smoothB = 0;

String colorName = "UNKNOWN";
String hexColor = "#000000";

int prevRed = -1000;
int prevGreen = -1000;
int prevBlue = -1000;

String prevHex = "";
String prevColor = "";

//====================================================
// RGB888 → RGB565
//====================================================

uint16_t rgb565(uint8_t r,uint8_t g,uint8_t b)
{
    return ((r & 0xF8)<<8) |
           ((g & 0xFC)<<3) |
            (b>>3);
}

//====================================================
// STATIC SCREEN
//====================================================

void drawStaticUI()
{
    gfx->fillScreen(BLACK);

    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);

    gfx->setCursor(75,12);
    gfx->print("SMART");

    gfx->setCursor(35,36);
    gfx->print("COLOR SCANNER");

    gfx->drawCircle(120,85,28,WHITE);

    gfx->setTextSize(1);

    gfx->setCursor(55,150);
    gfx->print("R:");

    gfx->setCursor(55,165);
    gfx->print("G:");

    gfx->setCursor(55,180);
    gfx->print("B:");
}

//====================================================
// SETUP
//====================================================

void setup()
{
    pinMode(S0,OUTPUT);
    pinMode(S1,OUTPUT);
    pinMode(S2,OUTPUT);
    pinMode(S3,OUTPUT);

    pinMode(SENSOR_OUT,INPUT);

    digitalWrite(S0,HIGH);
    digitalWrite(S1,LOW);

    gfx->begin();

    drawStaticUI();
}
//====================================================
// READ RED
//====================================================
int readRed()
{
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);

    return pulseIn(SENSOR_OUT, LOW);
}

//====================================================
// READ GREEN
//====================================================
int readGreen()
{
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);

    return pulseIn(SENSOR_OUT, LOW);
}

//====================================================
// READ BLUE
//====================================================
int readBlue()
{
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);

    return pulseIn(SENSOR_OUT, LOW);
}

//====================================================
// STABLE RGB READING
//====================================================
void updateRGB()
{
    long r = 0;
    long g = 0;
    long b = 0;

    const int samples = 10;

    for(int i = 0; i < samples; i++)
    {
        r += map(readRed(),   20,180,255,0);
        g += map(readGreen(), 20,180,255,0);
        b += map(readBlue(),  20,180,255,0);

        delay(2);
    }

    r /= samples;
    g /= samples;
    b /= samples;

    r = constrain(r,0,255);
    g = constrain(g,0,255);
    b = constrain(b,0,255);

    // Exponential smoothing
    smoothR = smoothR * 0.80 + r * 0.20;
    smoothG = smoothG * 0.80 + g * 0.20;
    smoothB = smoothB * 0.80 + b * 0.20;

    redValue   = (int)smoothR;
    greenValue = (int)smoothG;
    blueValue  = (int)smoothB;
}
//====================================================
// RGB -> HEX (Stable)
//====================================================
void updateHEX()
{
    // Ignore tiny RGB changes (less than 3 levels)
    if (abs(redValue - prevRed) < 3 &&
        abs(greenValue - prevGreen) < 3 &&
        abs(blueValue - prevBlue) < 3)
    {
        return;
    }

    char hexBuffer[8];

    sprintf(hexBuffer,
            "#%02X%02X%02X",
            redValue,
            greenValue,
            blueValue);

    hexColor = String(hexBuffer);
}
//====================================================
// UPDATE COLOR CIRCLE
//====================================================
void updateCircle()
{
    if (abs(redValue - prevRed) < 3 &&
        abs(greenValue - prevGreen) < 3 &&
        abs(blueValue - prevBlue) < 3)
    {
        return;
    }

    uint16_t color = rgb565(redValue, greenValue, blueValue);

    gfx->fillCircle(120, 85, 25, color);
    gfx->drawCircle(120, 85, 28, WHITE);

    prevRed   = redValue;
    prevGreen = greenValue;
    prevBlue  = blueValue;
}
//====================================================
// UPDATE COLOR NAME
//====================================================
void updateColorName()
{
    if (colorName == prevColor)
        return;

    // Clear only the color name area
    gfx->fillRect(20, 110, 200, 28, BLACK);

    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);

    // Better centering
    int x = (240 - (colorName.length() * 12)) / 2;

    gfx->setCursor(x, 118);
    gfx->print(colorName);

    prevColor = colorName;
}
//====================================================
// UPDATE RGB VALUES
//====================================================
void updateRGBText()
{
    // Don't redraw if values are almost identical
    if (abs(redValue - prevRed) < 3 &&
        abs(greenValue - prevGreen) < 3 &&
        abs(blueValue - prevBlue) < 3)
    {
        return;
    }

    // Clear only the numbers
    gfx->fillRect(75,145,70,45,BLACK);

    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);

    gfx->setCursor(75,150);
    gfx->print(redValue);

    gfx->setCursor(75,165);
    gfx->print(greenValue);

    gfx->setCursor(75,180);
    gfx->print(blueValue);
}
//====================================================
// UPDATE HEX
//====================================================
void updateHex()
{
    if (hexColor == prevHex)
        return;

    gfx->fillRect(20,195,200,28,BLACK);

    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);

    int x = (240 - (hexColor.length() * 12)) / 2;

    gfx->setCursor(x,205);
    gfx->print(hexColor);

    prevHex = hexColor;
}
//====================================================
// ADVANCED COLOR DETECTION
//====================================================
void detectColor()
{
    // Neutral colors
    if(redValue>220 && greenValue>220 && blueValue>220)
        colorName="WHITE";

    else if(redValue<35 && greenValue<35 && blueValue<35)
        colorName="BLACK";

    else if(abs(redValue-greenValue)<15 &&
            abs(redValue-blueValue)<15 &&
            redValue>170)
        colorName="SILVER";

    else if(abs(redValue-greenValue)<15 &&
            abs(redValue-blueValue)<15)
        colorName="GRAY";

    // Reds
    else if(redValue>190 && greenValue<80 && blueValue<80)
        colorName="RED";

    else if(redValue>120 && greenValue<40 && blueValue<40)
        colorName="MAROON";

    // Orange
    else if(redValue>200 &&
            greenValue>110 &&
            greenValue<190 &&
            blueValue<70)
        colorName="ORANGE";

    // Gold
    else if(redValue>180 &&
            greenValue>150 &&
            blueValue<70)
        colorName="GOLD";

    // Yellow
    else if(redValue>190 &&
            greenValue>190 &&
            blueValue<90)
        colorName="YELLOW";

    // Lime
    else if(greenValue>220 &&
            redValue>170 &&
            blueValue<90)
        colorName="LIME";

    // Green
    else if(greenValue>180 &&
            redValue<130 &&
            blueValue<130)
        colorName="GREEN";

    else if(greenValue>100 &&
            redValue<80 &&
            blueValue<80)
        colorName="DARK GREEN";

    // Cyan
    else if(greenValue>170 &&
            blueValue>170 &&
            redValue<120)
        colorName="CYAN";

    // Sky Blue
    else if(blueValue>190 &&
            greenValue>150 &&
            redValue<130)
        colorName="SKY BLUE";

    // Blue
    else if(blueValue>180 &&
            redValue<130 &&
            greenValue<130)
        colorName="BLUE";

    else if(blueValue>100 &&
            redValue<60 &&
            greenValue<60)
        colorName="NAVY";

    // Purple
    else if(redValue>150 &&
            blueValue>150 &&
            greenValue<100)
        colorName="PURPLE";

    else if(redValue>180 &&
            blueValue>180 &&
            greenValue<120)
        colorName="VIOLET";

    // Pink
    else if(redValue>220 &&
            blueValue>170 &&
            greenValue>120)
        colorName="PINK";

    // Brown
    else if(redValue>100 &&
            redValue<180 &&
            greenValue>50 &&
            greenValue<120 &&
            blueValue<70)
        colorName="BROWN";

    // Chocolate
    else if(redValue>120 &&
            greenValue>60 &&
            greenValue<90 &&
            blueValue<50)
        colorName="CHOCOLATE";

    // Beige
    else if(redValue>190 &&
            greenValue>180 &&
            blueValue>120)
        colorName="BEIGE";

    // Tan
    else if(redValue>170 &&
            greenValue>130 &&
            blueValue>80)
        colorName="TAN";

    // Olive
    else if(redValue>100 &&
            greenValue>100 &&
            blueValue<60)
        colorName="OLIVE";

    // Teal
    else if(greenValue>120 &&
            blueValue>120 &&
            redValue<80)
        colorName="TEAL";

    // Lavender
    else if(redValue>180 &&
            blueValue>200 &&
            greenValue>150)
        colorName="LAVENDER";

    // Coral
    else if(redValue>220 &&
            greenValue>120 &&
            blueValue>100)
        colorName="CORAL";

    else
        colorName="UNKNOWN";
}
//====================================================
// DOMINANT COLOR FILTER
//====================================================
String stableHex = "#000000";
String stableColor = "UNKNOWN";

int stableCounter = 0;

const int CHANGE_THRESHOLD = 5;   // Number of identical readings before accepting a new color
//====================================================
// RGB -> HEX (Dominant Stable)
//====================================================
void updateHEX()
{
    char hexBuffer[8];

    sprintf(hexBuffer,
            "#%02X%02X%02X",
            redValue,
            greenValue,
            blueValue);

    String newHex = String(hexBuffer);

    if(newHex == stableHex)
    {
        stableCounter = 0;
        hexColor = stableHex;
        return;
    }

    stableCounter++;

    if(stableCounter >= CHANGE_THRESHOLD)
    {
        stableHex = newHex;
        hexColor = stableHex;
        stableCounter = 0;
    }
}
void updateColorName()
{
    if(colorName != stableColor)
    {
        stableCounter++;

        if(stableCounter >= CHANGE_THRESHOLD)
        {
            stableColor = colorName;
            stableCounter = 0;
        }
    }

    if(stableColor == prevColor)
        return;

    gfx->fillRect(20,110,200,28,BLACK);

    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);

    int x = (240 - stableColor.length()*12)/2;

    gfx->setCursor(x,118);
    gfx->print(stableColor);

    prevColor = stableColor;
}
void updateCircle()
{
    static uint16_t lastColor = 0;

    uint16_t currentColor = rgb565(redValue,greenValue,blueValue);

    if(currentColor == lastColor)
        return;

    gfx->fillCircle(120,85,25,currentColor);

    gfx->drawCircle(120,85,28,WHITE);

    lastColor = currentColor;
}
void loop()
{
    updateRGB();

    detectColor();

    updateHEX();

    updateCircle();

    updateColorName();

    updateRGBText();

    updateHex();

    delay(50);
}