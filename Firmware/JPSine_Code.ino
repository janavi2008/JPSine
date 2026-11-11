#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <Adafruit_GFX.h>



const byte Rows[8]= {5, 6, 7, 8, 9, 10, 11, 12};
const byte Cols[5]= {0, 1, 2, 3, 4};
const byte numRows = 8;
const byte numCols = 5;
#define SDA_pin 14
#define SCL_pin 15
#define Battery 26
Adafruit_SSD1306 display(128, 64, &Wire1, -1);

bool shiftOn = false;
bool alphaOn = false;
bool degMode = true;
double ans = 0.0;
double Vars[7]={0};
String result = "0";

enum Token : byte {
  None = 0,
  Num_0, Num_1, Num_2, Num_3, Num_4, Num_5, Num_6, Num_7, Num_8, Num_9,
  Num_exp10, Decimal, 
  Op_add, Op_sub, Op_mul, Op_div, Op_pow, Op_neg,
  Func_sin, Func_cos, Func_tan, Func_asin, Func_acos, Func_atan, 
  Func_log, Func_ln, Func_10x, Func_ex,
  Func_sqroot, Func_cbroot, Func_root, 
  Post_fact, Post_sqr, Post_inv,
  Brack_open, Brack_close,
  Const_pi, Const_e, Const_ans,
  Var_A, Var_B, Var_C, Var_D, Var_E, Var_F, Var_X,
  Mem_sto, Mem_rcl,
  Contr_shift, Contr_alpha, Contr_mode, Contr_del, Contr_equals, Contr_ac, 
  Contr_L, Contr_R, Contr_U, Contr_D, 

  Tok_count
};

const char* tokenText[Tok_count] = {
  "", "0", "1", "2", "3", "4", "5","6", "7", "8", "9", "E", ".", 
  "+", "-", "*", "/", "^", "-", 
  "sin(", "cos(", "tan(", "asin(", "acos(", "atan(", "log(", "ln(", "10^(", "e^(", "sqrt(", "cbrt(", "root",
  "!", "^2", "^-1",
  "(", ")",
  "pi", "e", "ans",
  "A", "B", "C", "D", "E", "F", "X",
  "", "", "", "", "","", "", "", "","", "", ""
};


const Token keys[numRows][numCols] = { 
  {Contr_shift, Contr_alpha, Contr_U, Contr_mode, Mem_sto},
  {Post_sqr, Contr_L, Contr_D, Contr_R, Post_inv},
  {Func_sin, Func_cos, Func_tan, Func_log, Func_ln},
  {Func_sqroot, Op_pow, Op_neg, Brack_open, Brack_close},
  {Num_7, Num_8, Num_9, Contr_del, Contr_ac},
  {Num_4, Num_5, Num_6, Op_mul, Op_div},
  {Num_1, Num_2, Num_3, Op_add, Op_sub},
  {Num_0, Decimal, Num_exp10, Const_ans, Contr_equals}  

};

const Token keyShift[numRows][numCols]= {
  {Contr_shift, Contr_alpha, Contr_U, Contr_mode, Mem_rcl},
  {Func_cbroot, Contr_L, Contr_D, Contr_R, Post_fact},
  {Func_asin, Func_acos, Func_atan, Func_10x, Func_ex},
  {Func_root, None, None, Brack_open, Brack_close},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, None, None, Const_e, Const_pi},

};

const Token keyAlpha[numRows][numCols] = {
  {Contr_shift, Contr_alpha, Contr_U, Contr_mode, None},
  {None, Contr_L, Contr_D, Contr_R, None},
  {Var_A, Var_B, Var_C, Var_D, Var_E},
  {Var_F, Var_X, None, None, None},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, None, None, None, None},
};

bool lastKey[numRows][numCols] = {false};
unsigned long lastScan =0;
bool rclOn = false; 
bool stoOn = false;
Token expr[64];
int exprLen = 0;
int Cursor =0;

int BODMAS(Token t){
  if (t == Op_add || t ==Op_sub) return 1;
  if (t == Op_mul || t == Op_div) return 2;
  if (t == Op_neg) return 3;
  if (t == Op_pow || t == Func_root) return 4;
  return 0;
}


void Calculate(){
  if (exprLen == 0) return;
  Token rpnTok[128];
  double rpnVal[128];
  int rpnLen = 0;
  Token opStack[64];
  int opTop = -1;
  int i = 0;
  while(i < exprLen){
    Token t = expr[i];
    if (t >= Num_0 && t <= Decimal){
      String numStr = "";
      while(i < exprLen){
        Token n = expr[i];
        if (n >= Num_0 && n <= Num_9) numStr += char('0' + (n - 1));
        else if (n == Decimal) numStr += '.';
        else if (n == Num_exp10) numStr += 'e';
        else if ((n == Op_neg || n == Op_sub) && numStr.endsWith("e")) numStr += '-';
        else break;
        i++;
      }
      rpnTok[rpnLen] = Num_0;rpnVal[rpnLen] = numStr.toDouble();
      rpnLen++;
      continue;
    }
    if (t >=Const_pi && t <= Var_X){

    }

  }
}

void updateDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  if(shiftOn) display.print("[S]");
  if(alphaOn) display.print("[A]");
  if(stoOn) display.print("[STO]");
  if(rclOn) display.print("[RCL]");
  display.print(degMode ? "DEG" : "RAD");
  float BattVol = (analogRead(Battery)/4095.0)*3.3*2.0;
  display.setCursor(92, 0);
  display.print(BattVol, 1);
  display.print("V");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  for(byte i = 0; i < exprLen; i++){
    if(Cursor == i) display.print("|");
    display.print(tokenText[expr[i]]);
  }

  if (Cursor == exprLen) display.print("|");
  display.setTextSize(2);
  display.setCursor(0, 44);
  display.print(result);
  display.display();
}

void checkToken(Token t){
  if (t == None) return;
  if ((stoOn || rclOn) && t >= Var_A && t <= Var_X){
    int id = t - Var_A;
    if (stoOn){
      Vars[id] = ans;
      result = String(Vars[id], 5);
      stoOn = false;
    }
    if (rclOn){
      result = String(Vars[id], 5);
      rclOn = false;
    }
    shiftOn = false;
    alphaOn = false;
    updateDisplay();
    return;
  }

  switch(t){
    case Contr_shift: 
      shiftOn = !shiftOn;
      alphaOn = false;
      updateDisplay();
      return;
    case Contr_alpha:
      alphaOn = !alphaOn;
      shiftOn = false;
      updateDisplay();
      return;
    case Contr_mode:
      degMode = !degMode;
      break;
    case Mem_sto:
      stoOn = true;
      rclOn = false;
      break;
    case Mem_rcl:
      rclOn = true;
      stoOn = false;
      break;
    case Contr_L:
      if(Cursor > 0) Cursor--;
      break;
    case Contr_R:
      if(Cursor < exprLen) Cursor++;
      break;
    case Contr_del:
      if(Cursor > 0){
        for(int i = Cursor-1;  i < exprLen -1; i++){
          expr[i] = expr[i+1];
        }
        exprLen--;
        Cursor--;
      }
      break;
    case Contr_ac:
      exprLen = 0;
      Cursor = 0;
      result = "0";
      rclOn = false;
      stoOn = false;
      break;
    case Contr_equals:
      Calculate();
      break;
    default:
      if (exprLen < 63){
        for (int i = exprLen; i > Cursor; i--){
          expr[i] = expr[i-1];
        }
        expr[Cursor] = t;
        exprLen++;
        Cursor++;
      }
      break;
  }
  shiftOn = false;
  alphaOn = false;
  updateDisplay();
}


void setup() {

  pinMode(Battery, INPUT);
  analogReadResolution(12);
  Wire1.setSDA(SDA_pin);
  Wire1.setSCL(SCL_pin);
  Wire1.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  for (byte r = 0; r < numRows; r++){
    pinMode(Rows[r], OUTPUT);
    digitalWrite(Rows[r], HIGH);
  }
  for (byte c = 0; c < numCols; c++){
    pinMode(Cols[c], INPUT_PULLUP);
  }
  updateDisplay();
}

void loop() {

  if (millis() - lastScan > 20){
    lastScan = millis();
    for(byte r = 0; r < numRows; r++){
      digitalWrite(Rows[r], LOW);
      delayMicroseconds(5);
      for (byte c =0; c < numCols; c++){
        bool press = (digitalRead(Cols[c])== LOW);
        Token t = None;
        if (press && !lastKey[r][c]){
          t = keys[r][c];
          if (shiftOn && keyShift[r][c] != None) t = keyShift[r][c];
           else if (alphaOn && keyAlpha[r][c] != None) t = keyAlpha[r][c];
          checkToken(t);
        }
        lastKey[r][c] = press;
      }
      digitalWrite(Rows[r], HIGH);
    }
  }

}
