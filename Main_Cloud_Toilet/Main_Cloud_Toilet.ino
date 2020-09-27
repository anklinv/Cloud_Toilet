#include <math.h>
#include <stdint.h>

#define HSV_CHANNEL_MAX          255
#define HSV_CHANNEL_MIN          0
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _b : _a; })

// Pins
#define MAIN_R 5
#define MAIN_G 6
#define MAIN_B 3
#define ACC_R 10
#define ACC_G 11
#define ACC_B 9

// Parameters
#define MAIN 0
#define ACC 1

struct color_RGB {
   uint8_t     b;
   uint8_t     g;
   uint8_t     r;
};

struct color_HSV {
   uint8_t     h;
   uint8_t     s;
   uint8_t     v;
};

color_HSV current_main = {0, 0, 0};
color_HSV current_accent = {0, 0, 0};
int lightning[6] = {4, 2, 8, 7, 13, 12};

// Colors
color_HSV white = {0, 0, 100};
color_HSV silver = {0, 0, 75};
color_HSV gray = {0, 0, 50};
color_HSV black = {0, 0, 0};
color_HSV red = {0, 100, 100};
color_HSV maroon = {0, 100, 50};
color_HSV yellow = {43, 100, 100};
color_HSV olive = {43, 100, 50};
color_HSV lime = {86, 100, 100};
color_HSV green = {86, 100, 50};
color_HSV aqua = {128, 100, 100};
color_HSV teal = {128, 100, 50};
color_HSV blue = {171, 100, 100};
color_HSV navy = {171, 100, 50};
color_HSV fuchsia = {213, 100, 100};
color_HSV purple = {213, 100, 50};

int oldVal;

void setup() {
  Serial.begin(9600);

  // Setup pins
  pinMode(MAIN_R, OUTPUT);
  pinMode(MAIN_G, OUTPUT);
  pinMode(MAIN_B, OUTPUT);
  pinMode(ACC_R, OUTPUT);
  pinMode(ACC_G, OUTPUT);
  pinMode(ACC_B, OUTPUT);
  pinMode(lightning[0], OUTPUT);
  pinMode(lightning[1], OUTPUT);
  pinMode(lightning[2], OUTPUT);
  pinMode(lightning[3], OUTPUT);
  pinMode(lightning[4], OUTPUT);
  pinMode(lightning[5], OUTPUT);
  oldVal = analogRead(0);
}

void color_HSV2RGB(struct color_HSV const *hsv, struct color_RGB *rgb) {
   int i;
   float f,p,q,t;
   float h, s, v;

   //expand the u8 hue in range 0->255 to 0->359* (there are problems at exactly 360)
   h = 359.0 * ((float)hsv->h / 255.0);

   h = MAX(0.0, MIN(360.0, h));
   s = MAX(0.0, MIN(100.0, hsv->s));
   v = MAX(0.0, MIN(100.0, hsv->v));

   s /= 100;
   v /= 100;

   if(s == 0) {
      // Achromatic (grey)
      rgb->r = rgb->g = rgb->b = round(v*255);
      return;
   }

   h /= 60; // sector 0 to 5
   i = floor(h);
   f = h - i; // factorial part of h
   p = v * (1 - s);
   q = v * (1 - s * f);
   t = v * (1 - s * (1 - f));
   switch(i) {
      case 0:
         rgb->r = round(255*v);
         rgb->g = round(255*t);
         rgb->b = round(255*p);
         break;
      case 1:
         rgb->r = round(255*q);
         rgb->g = round(255*v);
         rgb->b = round(255*p);
         break;
      case 2:
         rgb->r = round(255*p);
         rgb->g = round(255*v);
         rgb->b = round(255*t);
         break;
      case 3:
         rgb->r = round(255*p);
         rgb->g = round(255*q);
         rgb->b = round(255*v);
         break;
      case 4:
         rgb->r = round(255*t);
         rgb->g = round(255*p);
         rgb->b = round(255*v);
         break;
      default: // case 5:
         rgb->r = round(255*v);
         rgb->g = round(255*p);
         rgb->b = round(255*q);
   }
}

void set_main(color_HSV color){
  color_RGB target;
  color_HSV2RGB(&color, &target);
  analogWrite(MAIN_R, target.r);
  analogWrite(MAIN_G, target.g);
  analogWrite(MAIN_B, target.b);
  current_main = color;
}

void set_accent(color_HSV color){
  color_RGB target;
  color_HSV2RGB(&color, &target);
  analogWrite(ACC_R, target.r);
  analogWrite(ACC_G, target.g);
  analogWrite(ACC_B, target.b);
  current_accent = color;
}

void fade(int total_time, color_HSV *current_color, color_HSV *target_color, void (*set_func)(color_HSV)) {
  int change_hue = ((int) current_color->h) - ((int) target_color->h);
  int change_saturation = ((int) current_color->s) - ((int) target_color->s);
  int change_value = ((int) current_color->v) - ((int) target_color->v);

  int frequency_hue = total_time / abs(change_hue);
  int frequency_saturation = total_time / abs(change_saturation);
  int frequency_value = total_time / abs(change_value);

  unsigned long last_hue_change = millis();
  unsigned long last_saturation_change = millis();
  unsigned long last_value_change = millis();

  unsigned long StartTime = millis();
  unsigned long CurrentTime = millis();
  unsigned long TargetFinishTime = StartTime + (unsigned long) total_time;

  do
  {
    CurrentTime = millis();
    
    if (last_hue_change - CurrentTime >= frequency_hue) {
      current_color->h += change_hue > 0 ? 1 : -1;
    }
        
    if (last_saturation_change - CurrentTime >= frequency_saturation) {
      current_color->s += change_saturation > 0 ? 1 : -1;
    }

    if (last_value_change - CurrentTime >= frequency_value) {
      current_color->v += change_value > 0 ? 1 : -1;
    }

    Serial.print("Change color to : (");
    Serial.print(current_color->h);
    Serial.print(",");
    Serial.print(current_color->s);
    Serial.print(",");
    Serial.print(current_color->v);
    Serial.print(")");
    Serial.print("\n");
    
    set_func(*current_color);
  } while (CurrentTime < TargetFinishTime);

  unsigned long actual_time = millis() - StartTime;
  Serial.print("Fade completed in ");
  Serial.print(actual_time);
  Serial.print(" expected ");
  Serial.print(total_time);
  Serial.print("\n");

  Serial.print("Final color is: (");
  Serial.print(current_color->h);
  Serial.print(",");
  Serial.print(current_color->s);
  Serial.print(",");
  Serial.print(current_color->v);
  Serial.print(")");
  Serial.print(" expected (");
  Serial.print(target_color->h);
  Serial.print(",");
  Serial.print(target_color->s);
  Serial.print(",");
  Serial.print(target_color->v);
  Serial.print(")");
  Serial.print("\n");
}

void fade_main(int total_time, color_HSV target_color) {
  fade(total_time, &current_main, &target_color, set_main);
}

void fade_accent(int total_time, color_HSV target_color) {
  fade(total_time, &current_accent, &target_color, set_accent);
}

void normal_lightning_wave(int min_delay, int small_change, bool backward = false){
  if (backward) {
    digitalWrite(lightning[0], HIGH);
    for (int i = 1; i < 6; ++i) {
      delay(min_delay + random(small_change));
      digitalWrite(lightning[i], HIGH);
      digitalWrite(lightning[i-1], LOW);
    }
    delay(min_delay + random(small_change));
    digitalWrite(lightning[5], LOW);
  } else {
    digitalWrite(lightning[5], HIGH);
    for (int i = 5; i >= 0; --i) {
      delay(min_delay + random(small_change));
      digitalWrite(lightning[i], HIGH);
      digitalWrite(lightning[i+1], LOW);
    }
    delay(min_delay + random(small_change));
    digitalWrite(lightning[0], LOW);
  }
}

void inward_lightning_wave(int min_delay, int small_change) {
  digitalWrite(lightning[0], HIGH);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[1], HIGH);
  digitalWrite(lightning[0], LOW);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[2], HIGH);
  digitalWrite(lightning[1], LOW);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[3], HIGH);
  digitalWrite(lightning[2], LOW);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[4], HIGH);
  digitalWrite(lightning[3], LOW);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[5], HIGH);
  digitalWrite(lightning[4], LOW);
  delay(min_delay + random(small_change));
  digitalWrite(lightning[5], LOW);
}

void show_config(int val, char* msg) {
  Serial.print("Value: ");
  Serial.print(val);
  Serial.print(" -> ");
  Serial.println(msg);
}

void strobo() {
  analogWrite(MAIN_R, 0);
  analogWrite(MAIN_G, 0);
  analogWrite(MAIN_B, 0);
  analogWrite(ACC_R, 0);
  analogWrite(ACC_G, 0);
  analogWrite(ACC_B, 0);
  int red = 0;
  for (float j = 512; j > 16; j /= 1.1) {
    for (int i = 0; i < 6; ++i) {
      digitalWrite(lightning[i], HIGH);
      delay(j);
      digitalWrite(lightning[i], LOW);
    }
  }
  analogWrite(MAIN_R, 255);
  analogWrite(MAIN_G, 255);
  analogWrite(MAIN_B, 255);
  analogWrite(ACC_R, 255);
  analogWrite(ACC_G, 0);
  analogWrite(ACC_B, 0);
  delay(200);
  analogWrite(MAIN_R, 0);
  analogWrite(MAIN_G, 0);
  analogWrite(MAIN_B, 0);
  analogWrite(ACC_R, 0);
  analogWrite(ACC_G, 0);
  analogWrite(ACC_B, 0);
  delay(1000);
  analogWrite(MAIN_R, 0);
  analogWrite(MAIN_G, 255);
  analogWrite(MAIN_B, 0);
  analogWrite(ACC_R, 255);
  analogWrite(ACC_G, 0);
  analogWrite(ACC_B, 255);
  delay(200);
  analogWrite(MAIN_R, 0);
  analogWrite(MAIN_G, 0);
  analogWrite(MAIN_B, 0);
  analogWrite(ACC_R, 0);
  analogWrite(ACC_G, 0);
  analogWrite(ACC_B, 0);
  delay(8000);
}

void gewitter() {
  // TODO
}

void romance() {
  // TODO
}

void rainbow() {
  // TODO
}

void xmas() {
  // TODO
}

void nervig() {
  // TODO
}

void chillig() {
  color_HSV color_1 = {floor((197 / 360) * 255), 50, 81};
  color_HSV color_2 = {floor((197 / 360) * 255), 37, 91};
  color_HSV color_3 = {floor((197 / 360) * 255), 20, 94};
  color_HSV color_4 = {floor((186 / 360) * 255), 8, 94};

  set_main(color_1);
  set_accent(color_1);
  delay(5000);
  fade_accent(2000, color_2);
  delay(5000);
  fade_main(2000, color_2);
  delay(5000);
  fade_accent(2000, color_3);
  delay(5000);
  fade_main(2000, color_3);
  delay(5000);
  fade_accent(2000, color_4);
  delay(5000);
  fade_main(2000, color_4);
  delay(5000);
  fade_accent(2000, color_1);
  delay(5000);
  fade_main(2000, color_1);
  delay(5000);
}

void loop() {
  int val = analogRead(0);    // read the value from the sensor
  uint8_t target_hue = map(val, 0, 1023, 0, 255);
  show_config(target_hue, "hue");
  color_HSV target_color = {target_hue, 100, 100};
  set_main(target_color);
  set_accent(target_color);
  delay(100);
  return;
  
  if (val > 900) {
    strobo();
  } else {
    set_main(white);
    set_accent(white);
    delay(500);
    set_main(silver);
    delay(500);
    set_main(gray);
    delay(500);
    set_main(black);
    delay(500);
    set_main(red);
    delay(500);
    set_main(maroon);
    delay(500);
    set_main(yellow);
    delay(500);
    set_main(olive);
    delay(500);
    set_main(lime);
    delay(500);
    set_main(green);
    delay(500);
    set_main(aqua);
    delay(500);
    set_main(blue);
    delay(500);
    set_main(navy);
    delay(500);
    set_main(fuchsia);
    delay(500);
    set_main(purple);
    delay(500);
    set_main(white);
    set_accent(white);
    delay(500);
    set_accent(silver);
    delay(500);
    set_accent(gray);
    delay(500);
    set_accent(black);
    delay(500);
    set_accent(red);
    delay(500);
    set_accent(maroon);
    delay(500);
    set_accent(yellow);
    delay(500);
    set_accent(olive);
    delay(500);
    set_accent(lime);
    delay(500);
    set_accent(green);
    delay(500);
    set_accent(aqua);
    delay(500);
    set_accent(blue);
    delay(500);
    set_accent(navy);
    delay(500);
    set_accent(fuchsia);
    delay(500);
    set_accent(purple);
  }
//  set_main(main_color);
//  set_accent(accent_color);
//  Serial.println(val);
//  if ((oldVal > val && (oldVal - val) >= 30) || (val > oldVal && (val - oldVal) >= 30)) {
//    oldVal = val;
//    int main_color[3] = {255, 0, 0};
//    int accent_color[3] = {255, 0, 0};
//    set_main(main_color);
//    set_accent(accent_color);
//    Serial.print("Main: ");
//    Serial.print("R: ");
//    Serial.print(main_color[0]);
//    Serial.print(" G: ");
//    Serial.print(main_color[1]);
//    Serial.print(" B: ");
//    Serial.println(main_color[2]);
//    Serial.print("Accent: ");
//    Serial.print("R: ");
//    Serial.print(accent_color[0]);
//    Serial.print(" G: ");
//    Serial.print(accent_color[1]);
//    Serial.print(" B: ");
//    Serial.println(accent_color[2]);
//  }
  
//  if (val >= 0 && val < 256) {
//    show_config(val, "Forward lightning");
//    normal_lightning_wave(val, 50);
//  } else if (val >= 256 && val < 512) {
//    show_config(val, "Backward lightning");
//    normal_lightning_wave(val - 256, 50, true);
//  } else if (val >= 512 && val < 768) {
//    show_config(val, "Inward lightning");
//    inward_lightning_wave(val - 512, 50);
//  } else if (val >= 786 && val < 1024) {
//    show_config(val, "Base colors");
//    if (val < 850) {
//      set_main(white);
//      set_accent(light_violet);
//    } else if (val >= 850 && val < 950) {
//      set_main(blue_gray);
//      set_accent(lighter_blue);
//    } else if (val >= 950 && val < 1000) {
//      set_main(brown_orange);
//      set_accent(red);
//    } else {
//      set_main(green);
//      set_accent(blue);
//    }
//  }
}
