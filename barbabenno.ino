/*
Adafruit Arduino - Lesson 3. RGB LED
*/

int redPin = 11;
int greenPin = 10;
int bluePin = 9;

const int micPin = A0;
const int drempel = 5; // Stel hiermee de gevoeligheid in. Lager is gevoeliger, minimumwaarde is 2.

int value1 = 0;
int value2 = 0;

#define COMMON_ANODE

const int len_signal = 100;

inline static float sqr(float x) {
    return x*x;
}

// Struct 
struct AudioSignal {
    int array[len_signal];
} ;
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

  public:
    Barbabenno()
    {
      pinMode(redPin, OUTPUT);
      pinMode(greenPin, OUTPUT);
      pinMode(bluePin, OUTPUT);  
      BarbaState = 1;
      setColor(BarbaState);
    };

    void Update(float signal_variance);
    void SetState(int BarbaState);
};

void Barbabenno::Update(float signal_variance) {
    if((signal_variance > 50) && (signal_variance < 700)) {
      if(BarbaState+1 < n_states){
        BarbaState = BarbaState + 1;
      } else {BarbaState = 0;}
      setColor(BarbaState);
    }
    // Hard klappen discomodus!
    if(signal_variance >= 700) {
      Serial.print("Discomodus");
      Serial.println();
      for(int i = 0; i < n_states; i++){
        setColor(i); 
        delay(200);
        setColor(n_states-1);
        delay(200);
      }
      for(int i = n_states-2; i > 0; i--){
        setColor(i); 
        delay(200);
        setColor(n_states-1);
        delay(200);
      }
    }
    
};

void Barbabenno::SetState(int BarbaState){
    setColor(BarbaState);
};


Barbabenno bb;


void setup() 
{

  Serial.begin(9600); 
  //setColor(0, 100, 0); 
  //setColor(4);  
  //delayMicroseconds(1000);
  //bb.SetState(4);

}
 
void loop()
{
  AudioSignal my_as;
  float signal_variance;
  value1 = analogRead(micPin);
  delayMicroseconds(100);
  value2 = analogRead(micPin);
  if (abs(value2 - value1) >= drempel) {   
    Serial.print("value1: ");
    Serial.print(value1);
    Serial.println();
    Serial.print("value2: ");
    Serial.print(value2);
    Serial.println();
    my_as = readSignal(10);
    signal_variance = CalcVariance(my_as);
    Serial.print("signal variance: ");
    Serial.print(signal_variance);
    Serial.println();
    bb.Update(signal_variance);
    //writeSignal(my_as);
//    if(signal_variance > 50) {
//      //setColor(100, 0, 0); 
//      //setColor(0);
//      bb.SetState(0);
//    } else {
//     //setColor(0, 0, 100); 
//      //setColor(1);
//      bb.SetState(1);
//    }
    delay(500);
    //setColor(4); 
    //bb.SetState(4);
  }
}

void setColor(int BarbaState)
//void setColor(int red, int green, int blue)
{
  int red = BarbaStateColors[BarbaState][0];
  int green = BarbaStateColors[BarbaState][1];
  int blue = BarbaStateColors[BarbaState][2];
  
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}


// sample n keer met delay_time
struct AudioSignal readSignal(int delay_time){
  AudioSignal my_as;
  for(int i = 0; i < len_signal; i++){
    my_as.array[i] = analogRead(micPin);
    delayMicroseconds(delay_time);
  }
  return my_as;
}

void writeSignal(AudioSignal my_as){
  for (int i = 0; i < len_signal; i++) {
  Serial.println(my_as.array[i]);
  }
}

float CalcVariance(AudioSignal my_as){
  // calculate mean
  float Mean = 0;
  
  for (int i = 0; i < len_signal; i++) {
    Mean = Mean + (float) my_as.array[i];
  }
  Mean = Mean/(float) len_signal;
  
  // calculate variance
  float avgVariance = 0;

  for (int i = 0; i < len_signal; i++) {
    avgVariance = avgVariance + sqr(Mean - my_as.array[i]);
  }
  avgVariance = avgVariance/(float) (len_signal - 1);
  return avgVariance;
}

