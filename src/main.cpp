#include <M5Stack.h>
#include <vector>

// 受信データ保存用
std::vector<float> data;
static int prev_n = 0;

// グラフ枠・軸・目盛・タイトル描画
void drawGraphFrame(int w, int h, int graph_x0, int graph_y0, int graph_w, int graph_h)
{
  // 枠・軸
  M5.Lcd.drawRect(graph_x0, graph_y0, graph_w, graph_h, WHITE);                                 // 枠
  M5.Lcd.drawLine(graph_x0, graph_y0, graph_x0, graph_y0 + graph_h, WHITE);                     // Y軸
  M5.Lcd.drawLine(graph_x0, graph_y0 + graph_h, graph_x0 + graph_w, graph_y0 + graph_h, WHITE); // X軸

  // Y軸目盛・ラベル
  for (int i = 0; i <= 4; ++i)
  {
    int y = graph_y0 + graph_h - (graph_h * i / 4);
    M5.Lcd.drawLine(graph_x0 - 5, y, graph_x0 + graph_w, y, 0xC618); // 薄いグレー目盛
    M5.Lcd.setTextColor(WHITE, 0x2104);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, y - 7);
    M5.Lcd.printf("%2.1f", 40.0f * i / 4);
  }

  // タイトル
  M5.Lcd.setTextColor(WHITE, 0x2104);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(graph_x0, 5);
  M5.Lcd.println("Serial Data Plotter");
}

void setup()
{
  M5.begin();
  M5.Lcd.fillScreen(0x2104); // 薄いグレー背景
  Serial.begin(115200);      // シリアル初期化

  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  int graph_x0 = 40, graph_y0 = 30, graph_w = w - 60, graph_h = h - 60;

  drawGraphFrame(w, h, graph_x0, graph_y0, graph_w, graph_h);
}

void loop()
{
  M5.update();
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  int graph_x0 = 40, graph_y0 = 30, graph_w = w - 60, graph_h = h - 60;

  // ボタンCでグラフリセット
  if (M5.BtnC.wasPressed())
  {
    data.clear();
    M5.Lcd.fillRect(graph_x0 + 1, graph_y0 + 1, graph_w - 2, graph_h - 2, 0x2104); // グラフ領域のみクリア
    M5.Lcd.setTextColor(WHITE, 0x2104);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(graph_x0 + 10, h / 2 - 10);
    M5.Lcd.println("Waiting for data...");
    prev_n = 0;
    return;
  }

  // シリアル受信
  if (Serial.available())
  {
    float value = Serial.parseFloat();
    Serial.print("Echo: ");
    Serial.println(value);
    if (value != 0.0f || Serial.peek() == '\n')
    {
      data.push_back(value);
      if (data.size() > 30)
        data.erase(data.begin()); // 最大30点
    }
  }

  int n = data.size();

  // データ数が変化したときのみ描画
  if (n == prev_n)
  {
    delay(20);
    return;
  }
  prev_n = n;

  M5.Lcd.fillRect(0, 0, w, h, 0x2104); // 枠線も含めてクリア
  drawGraphFrame(w, h, graph_x0, graph_y0, graph_w, graph_h);

  // データ折れ線
  if (n > 1)
  {
    for (int i = 1; i < n; ++i)
    {
      int x0 = graph_x0 + graph_w * (i - 1) / (n - 1);
      int y0 = graph_y0 + graph_h - (int)(graph_h * data[i - 1] / 40.0f);
      int x1 = graph_x0 + graph_w * i / (n - 1);
      int y1 = graph_y0 + graph_h - (int)(graph_h * data[i] / 40.0f);
      M5.Lcd.drawLine(x0, y0, x1, y1, GREEN); // 緑線
      M5.Lcd.fillCircle(x1, y1, 3, GREEN);    // 点を太く
    }
    // 最新値を赤丸で
    int x = graph_x0 + graph_w * (n - 1) / (n - 1);
    int y = graph_y0 + graph_h - (int)(graph_h * data[n - 1] / 40.0f);
    M5.Lcd.fillCircle(x, y, 6, RED);
  }
  else
  {
    // データが無い場合はメッセージ表示
    M5.Lcd.setTextColor(WHITE, 0x2104);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(graph_x0 + 10, h / 2 - 10);
    M5.Lcd.println("Waiting for data...");
  }
  delay(20);
}
