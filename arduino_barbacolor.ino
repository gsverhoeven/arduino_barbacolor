/*
Barbacolor: make Barbabenno cycle through rainbow colors using an Arduino Nano, an electret microphone and a RGB LED.
*/

int redPin = 11;
int greenPin = 10;
int bluePin = 9;

const int micPin = A0;
const int drempel = 5; //Sensitivity

int value1 = 0;
int value2 = 0;

const int n_intensities = 24;
const int stepTime = 100; // cycle time intensities in ms

float barbaIntensityScales[n_intensities] = {0, 0.02, 0.05, 0.1, 0.18, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 0.9, 1.0, 
0.9, 0.8, 0.6, 0.5, 0.4, 0.3, 0.2, 0.18, 0.1, 0.05, 0.02};

#define COMMON_ANODE

const int len_signal = 100;

inline static float sqr(float x) {
    return x*x;
}

// Struct to store Audio signal in 
struct AudioSignal {
    int array[len_signal];
} ;

// Rainbow colors from wikipedia

//Red (web color) (Hex: #FF0000) (RGB: 255, 0, 0)
//Orange (color wheel Orange) (Hex: #FF7F00) (RGB: 255, 127, 0)
//Yellow (web color) (Hex: #FFFF00) (RGB: 255, 255, 0)
//Green (X11) (Electric Green) (HTML/CSS “Lime”) (Color wheel green) (Hex: #00FF00) (RGB: 0, 255, 0)
//Blue (web color) (Hex: #0000FF) (RGB: 0, 0, 255)
//Violet (Electric Violet) (Hex: #8B00FF) (RGB: 139, 0, 255)

const int n_states = 6;

byte BarbaStateColors[n_states+1][3]={
 {255,0,0},
 {255,127,0},
 {255,255,0},
 {0,255,0},
 {0,0,255},
 {139,0,255},
 {0, 0, 0}
};

class Barbabenno
{
  // Class Member Variables
  int BarbaState;     
  int barbaIntensityState;
  unsigned long previousMillis;
  
  public:
    Barbabenno()
    {
      pinMode(redPin, OUTPUT);
      pinMode(greenPin, OUTPUT);
      pinMode(bluePin, OUTPUT);  
      BarbaState = 1;
      barbaIntensityState = 7;
      previousMillis = 0;
      setColor();
    };

    void Update(float signal_variance);
    void SetState();
    void UpdateIntensity(unsigned long currentMillis);
    void setColor();
};

void Barbabenno::Update(float signal_variance) {
    if((signal_variance > 50) && (signal_variance < 700)) {
      if(BarbaState+1 < n_states){
        BarbaState = BarbaState + 1;
      } else {BarbaState = 0;}
      Barbabenno::setColor();
    }
    // Really loud clap: discomodus!
    if(signal_variance >= 700) {
      Serial.print("Discomodus");
      Serial.println();
      barbaIntensityState = 12;
      for(int i = 0; i < n_states; i++){
        BarbaState = i;
        Barbabenno::setColor(); 
        delay(200);
        // turn off
        BarbaState = n_states-1;
        Barbabenno::setColor();
        delay(200);
      }
      for(int i = n_states-2; i > 0; i--){
        BarbaState = i;
        Barbabenno::setColor(); 
        delay(200);
        BarbaState = n_states-1;
        Barbabenno::setColor();
        delay(200);
      }
    }
    
};

void Barbabenno::SetState(){
    Barbabenno::setColor();
};

void Barbabenno::UpdateIntensity(unsigned long currentMillis){
    if(currentMillis - previousMillis > stepTime) {
      previousMillis = currentMillis;  
      if(barbaIntensityState+1 < n_intensities){
        barbaIntensityState = barbaIntensityState + 1;
      } else {barbaIntensityState = 0;}
      Barbabenno::setColor();
    }
};

void Barbabenno::setColor(){
  int red = BarbaStateColors[BarbaState][0];
  int green = BarbaStateColors[BarbaState][1];
  int blue = BarbaStateColors[BarbaState][2];

  // scale rgb's according to barbaIntensityState
  red = (int) ((float) red * barbaIntensityScales[barbaIntensityState]);
  Serial.print("red: ");
  Serial.print(red);
  green = (int) ((float) green * barbaIntensityScales[barbaIntensityState]);
  Serial.print("green: ");
  Serial.print(green);
  blue = (int) ((float) blue * barbaIntensityScales[barbaIntensityState]);
  Serial.print("blue: ");
  Serial.print(blue);

  #ifdef COMMON_ANODE
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  #endif
  
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

Barbabenno bb;

void setup() 
{
  Serial.begin(9600); // for debugging / calibration
}
 
void loop()
{
  AudioSignal my_as;
  float signal_variance;
  
  // detect start of sound
  value1 = analogRead(micPin);
  delayMicroseconds(100);
  value2 = analogRead(micPin);
  unsigned long currentMillis = millis();
  bb.UpdateIntensity(currentMillis); // check if intensity needs to be changed (Every onTime ms)
    
  if (abs(value2 - value1) >= drempel) {   
  
    Serial.print("value1: ");
    Serial.print(value1);
    Serial.println();
    Serial.print("value2: ");
    Serial.print(value2);
    Serial.println();

    // Take longer audio signal for loudness detection
    my_as = readSignal(10);
    // calculate "loudness" (variance of the signal)
    signal_variance = CalcVariance(my_as);
    
    Serial.print("signal variance: ");
    Serial.print(signal_variance);
    Serial.println();
    bb.Update(signal_variance);
    delay(500);
  }
}

// sample n times met delay_time
struct AudioSignal readSignal(int delay_time){
  AudioSignal my_as;
  for(int i = 0; i < len_signal; i++){
    my_as.array[i] = analogRead(micPin);
    delayMicroseconds(delay_time);
  }
  return my_as;
}

// write signal to serial line for debugging
void writeSignal(AudioSignal my_as){
  for (int i = 0; i < len_signal; i++) {
  Serial.println(my_as.array[i]);
  }
}

// calculate variance of signal ("loudness")
float CalcVariance(AudioSignal my_as){
  // first calculate mean
  float Mean = 0;
  
  for (int i = 0; i < len_signal; i++) {
    Mean = Mean + (float) my_as.array[i];
  }
  Mean = Mean/(float) len_signal;
  
  // then calculate variance
  float avgVariance = 0;

  for (int i = 0; i < len_signal; i++) {
    avgVariance = avgVariance + sqr(Mean - my_as.array[i]);
  }
  avgVariance = avgVariance/(float) (len_signal - 1);
  return avgVariance;
}


