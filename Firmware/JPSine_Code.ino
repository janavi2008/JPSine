#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <Adafruit_GFX.h>

const byte numRows = 8;
const byte numCols = 5;
const byte Rows[numRows]= {5, 6, 7, 8, 9, 10, 11, 12};
const byte Cols[numCols]= {0, 1, 2, 3, 4};
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

enum Token : byte { //all functions of the calc
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

const char* tokenText[Tok_count] = { // text that gets displayed when its button pressed
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

const Token keyShift[numRows][numCols]= { //keys when shift on
  {Contr_shift, Contr_alpha, Contr_U, Contr_mode, Mem_rcl},
  {Func_cbroot, Contr_L, Contr_D, Contr_R, Post_fact},
  {Func_asin, Func_acos, Func_atan, Func_10x, Func_ex},
  {Func_root, None, None, Brack_open, Brack_close},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, None, None, None, None},
  {None, Const_e, Const_pi, None, None, },

};

const Token keyAlpha[numRows][numCols] = { //keys when alpha on
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

int BODMAS(Token t){ //priority of operations
  if (t == Op_add || t ==Op_sub) return 1;
  if (t == Op_mul || t == Op_div) return 2;
  if (t == Op_neg) return 3;
  if (t == Op_pow || t == Func_root) return 4;
  return 0;
}

enum Kind :byte {K_NUM, K_BIN, K_PRE, K_POST, K_FUNC, K_OBRACK, K_CBRACK };
enum Err  : byte {E_OK, E_SYNTAX, E_MATH };

struct Item {
  Kind kind;
  Token tok;
  double val;
};

Item items[128];
Item rpn[128];     


bool addItem(Item list[], int &n, Kind k, Token t, double v) {
  if (n >=128) return false;
  list[n].kind = k;  
  list[n].tok = t;
  list[n].val = v;
  n++;
  return true;
}

Err parseItem(double xVal, int &n) { //converts buttons pressed to items and stores in array
  n = 0;
  int i = 0;
  while (i < exprLen) {
    Token t = expr[i];
    if (t >= Num_0 && t <= Decimal) {
      String NumStr = "";
      while(i < exprLen){
        Token d = expr[i];
        if (d >= Num_0 && d <= Num_9) NumStr += char('0' + (d - 1));
        else if (d == Decimal) NumStr += '.';
        else if (d== Num_exp10) NumStr += 'e';
        else if ((d == Op_neg || d == Op_sub) && NumStr.endsWith("e")) NumStr += '-';
        else break;
        i++;
      }
      addItem(items, n, K_NUM, None, NumStr.toDouble());
      continue;
    }
    if (t == Const_pi)  { addItem(items, n, K_NUM, None, M_PI); i++; continue; }
    if (t == Const_e)   { addItem(items, n, K_NUM, None, M_E); i++; continue; }
    if (t == Const_ans) { addItem(items, n, K_NUM, None, ans); i++; continue; }
    if (t >= Var_A && t <= Var_X)    { 
      double v = (t == Var_X) ? xVal : Vars[t - Var_A];
      addItem(items, n, K_NUM, None, v); i++; continue; }
    if (t >= Op_add && t <= Op_pow) || t == Func_root)    {addItem(items, n, K_BIN, t, 0); i++; continue; }
    if (t == Op_neg)    { addItem(items, n, K_PRE, t, 0); i++; continue; }
    if (t >= Post_fact && t <= Post_inv)   {addItem(items, n, K_POST, t, 0); i++; continue; }
    if (t >= Func_sin && t <= Func_cbroot)   { addItem(items, n, K_FUNC, t, 0); i++; continue; }
    if (t == Brack_open)  { addItem(items, n, K_OBRACK, None, 0); i++; continue; }
    if (t == Brack_close) { addItem(items, n, K_CBRACK, None, 0); i++; continue; }
    return E_SYNTAX;
  }
  return E_OK;
}

Item spaced[128];
bool endsValue(const Item &it)   { 
  return it.kind == K_NUM || it.kind == K_CBRACK || it.kind == K_POST; 
}
bool startsValue(const Item &it) { 
  return it.kind == K_NUM || it.kind == K_OBRACK || it.kind == K_FUNC; 
}

Err insertMultiply(int n, int &m) { //multiplies values that are next to eachothr  
  m = 0;
  for (int i = 0; i < n; i++) {
    if (i > 0 && endsValue(items[i-1]) && startsValue(items[i])) {
      if (!addItem(spaced,m, K_BIN, Op_mul, 0)) return E_SYNTAX;
    }
    if (!addItem(spaced, m, items[i].kind, items[i].tok,items[i].val)) return E_SYNTAX;
  }
   return E_OK;
}

Item stk[64];  

bool rightAssoc(Token t) { return t == Op_pow || t == Func_root; }

Err toRPN(int m, int &r) { //converts input expression to rpn so it can be evaluated
  r = 0;
   int top = -1;
  for (int i = 0; i < m; i++) {
    Item it = spaced[i];
    switch (it.kind) {
      case K_NUM:
      case K_POST:
        if (!addItem(rpn, r, it.kind, it.tok, it.val)) return E_SYNTAX;
        break;
      case K_PRE:
      case K_FUNC:
      case K_OBRACK:
        if (top >= 63) return E_SYNTAX;
        stk[++top] = it;
        break;
      case K_BIN:
        while (top >= 0 && (stk[top].kind == K_BIN || stk[top].kind == K_PRE)) {
          int pTop = BODMAS(stk[top].tok);
          int pCur = BODMAS(it.tok);
          if (pTop > pCur || (pTop == pCur && !rightAssoc(it.tok))) {
            if (!addItem(rpn, r, stk[top].kind, stk[top].tok, stk[top].val)) return E_SYNTAX;
            top--;
          } else break;
        }
        if (top >= 63) return E_SYNTAX;
        stk[++top] = it;
        break;
      case K_CBRACK:
        while (top >= 0 && stk[top].kind != K_OBRACK && stk[top].kind != K_FUNC) {
          if (!addItem(rpn, r, stk[top].kind, stk[top].tok, stk[top].val)) return E_SYNTAX;
          top--;
        }
        if (top < 0) return E_SYNTAX;
        if (stk[top].kind == K_FUNC) {
          if (!addItem(rpn, r, stk[top].kind, stk[top].tok, stk[top].val)) return E_SYNTAX;
        }
        top--;
        break;
    }
  }

  while (top >= 0) {
    if (stk[top].kind != K_OBRACK) {
      if (!addItem(rpn, r, stk[top].kind, stk[top].tok, stk[top].val)) return E_SYNTAX;
    }
    top--;
  }
  return E_OK;
}
double vstk[64];

double toRad(double x)   { return degMode ? x * M_PI / 180.0 : x; }
double fromRad(double x) { return degMode ? x * 180.0 / M_PI : x; }
double clean(double x)   { return fabs(x) < 1e-14 ? 0.0 : x; }

bool factorial(double x, double &res) {
  if (x < 0 || x != floor(x) || x > 170) return false;
  res = 1;
  for (int k = 2; k <= (int)x; k++) res *= k;
  return true;
}

Err evalRPN(int r, double &out) { //does actual calculator stuff
  int top = -1;
  for (int i = 0; i< r; i++) {
    Item it = rpn[i];
    double res = 0;

    if (it.kind == K_NUM) {
      res = it.val;
    }
    else if (it.kind== K_BIN) {
      if (top < 1) return E_SYNTAX;
      double b = vstk[top--];
      double a = vstk[top--];
      switch (it.tok) {
        case Op_add: res = a+b; break;
        case Op_sub: res = a-b; break;
        case Op_mul: res = a*b; break;
        case Op_div: 
          if (b == 0) return E_MATH; 
          res = a/b; break;
        case Op_pow: res = pow(a, b); break;
        case Func_root:
          if (a == 0) return E_MATH;
          if (b >= 0) res = pow(b, 1.0 / a);
          else if (a == floor(a) && fmod(a, 2) != 0) res = -pow(-b, 1.0 / a);
          else return E_MATH;
          break;
        default: return E_SYNTAX;
      }
    }
    else if (it.kind == K_PRE) {
      if (top < 0) return E_SYNTAX;
      res = -vstk[top--];
    }
    else if (it.kind == K_POST) {
      if (top < 0) return E_SYNTAX;
      double x = vstk[top--];
      switch (it.tok) {
        case Post_fact: if (!factorial(x, res)) return E_MATH; break;
        case Post_sqr: res = x*x; break;
        case Post_inv: if (x == 0) return E_MATH; res =1.0/x; break;
        default: return E_SYNTAX;
      }
    }
    else if (it.kind == K_FUNC) {
      if (top < 0) return E_SYNTAX;
      double x = vstk[top--];
      switch (it.tok) {
        case Func_sin: res = clean(sin(toRad(x))); break;
        case Func_cos: res = clean(cos(toRad(x))); break;
        case Func_tan:
          if (degMode && fabs(fmod(x, 180.0)) == 90.0) return E_MATH;
          res = clean(tan(toRad(x))); break;
        case Func_asin: if (x < -1 || x > 1) return E_MATH; res = fromRad(asin(x)); break;
        case Func_acos: if (x < -1 || x > 1) return E_MATH; res = fromRad(acos(x)); break;
        case Func_atan: res = fromRad(atan(x)); break;
        case Func_log:  if (x <= 0) return E_MATH; res = log10(x); break;
        case Func_ln:   if (x <= 0) return E_MATH; res = log(x); break;
        case Func_10x:  res = pow(10.0, x); break;
        case Func_ex:   res = exp(x); break;
        case Func_sqroot: if (x < 0) return E_MATH; res = sqrt(x); break;
        case Func_cbroot: res = cbrt(x); break;
        default: return E_SYNTAX;
      }
    }
    else return E_SYNTAX;
    if (isnan(res) || isinf(res)) return E_MATH;
    if (top >= 63) return E_SYNTAX;
    vstk[++top] = res;
  }
  if (top != 0) return E_SYNTAX;
  out = vstk[0];
  return E_OK;
}
String formatNumber(double v) { //turns the result to text that can be output
  if (v == 0) v = 0;
  char buf[24];
  snprintf(buf, sizeof(buf), "%.9g", v);
  return String(buf);
}

void Calculate() {
  if (exprLen == 0) return;
  int n = 0, m = 0, r = 0;
  double out = 0;
  Err e = parseItem(0, n);
  if (e == E_OK) e = insertMultiply(n, m);
  if (e == E_OK) e = toRPN(m, r);
  if (e == E_OK) e = evalRPN(r, out);
  if (e == E_OK) { ans = out; result = formatNumber(out); }
  else result = (e == E_MATH) ? "Math Error" : "Syntax Error";
}

void updateDisplay(){ //draws display
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

void checkPress(Token t){ //checks what the button press does
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
    case Contr_U: break;
    case Contr_D: break;
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
          checkPress(t);
        }
        lastKey[r][c] = press;
      }
      digitalWrite(Rows[r], HIGH);
    }
  }

}
