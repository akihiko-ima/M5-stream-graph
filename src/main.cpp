#include <deque>
#include <M5Stack.h>
#include <algorithm>

#define MAXIMUM_DATA_POINTS 10
#define TARGET_VALUE 23.0f

std::deque<float> data;
std::deque<float> prev_data;
static int prev_n = 0;

void drawGraphFrame(int w, int h, int graph_x0, int graph_y0, int graph_w, int graph_h)
{
  M5.Lcd.drawRect(graph_x0, graph_y0, graph_w, graph_h, WHITE);
  M5.Lcd.drawLine(graph_x0, graph_y0, graph_x0, graph_y0 + graph_h, WHITE);
  M5.Lcd.drawLine(graph_x0, graph_y0 + graph_h, graph_x0 + graph_w, graph_y0 + graph_h, WHITE);

  for (int i = 0; i <= 4; ++i)
  {
    int y = graph_y0 + graph_h - (graph_h * i / 4);
    M5.Lcd.drawLine(graph_x0 - 5, y, graph_x0 + graph_w, y, 0xC618);
    M5.Lcd.setTextColor(WHITE, 0x2104);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, y - 7);
    M5.Lcd.printf("%2.1f", 40.0f * i / 4);
  }

  M5.Lcd.setTextColor(WHITE, 0x2104);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(graph_x0, 5);
  M5.Lcd.println("Serial Data Plotter");
}

void setup()
{
  M5.begin();
  M5.Lcd.fillScreen(0x2104);
  Serial.begin(115200);
  Serial.setTimeout(10); // タイムアウト短縮

  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  drawGraphFrame(w, h, 40, 30, w - 60, h - 60);
}

void loop()
{
  M5.update();
  int w = M5.Lcd.width();
  int h = M5.Lcd.height();
  int graph_x0 = 40, graph_y0 = 30, graph_w = w - 60, graph_h = h - 60;

  if (M5.BtnC.wasPressed())
  {
    data.clear();
    M5.Lcd.fillRect(graph_x0 + 1, graph_y0 + 1, graph_w - 2, graph_h - 2, 0x2104);
    M5.Lcd.setCursor(graph_x0 + 10, h / 2 - 10);
    M5.Lcd.println("Waiting for data...");
    prev_n = 0;
    return;
  }

  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line == "data")
    {
      // dataコマンド受信時に配列を返す
      Serial.print("data: ");
      for (size_t i = 0; i < data.size(); ++i)
      {
        Serial.print(data[i]);
        if (i != data.size() - 1)
          Serial.print(",");
      }
      Serial.println();
    }
    else if (!line.isEmpty())
    {
      float value = line.toFloat();
      data.push_back(value);
      if (data.size() > MAXIMUM_DATA_POINTS)
        data.pop_front();
    }
  }
  // ちらつき対策
  // 中身が変化していなければreturn
  if (data == prev_data)
  {
    delay(20);
    return;
  }
  // 前回データを更新
  prev_data = data;

  M5.Lcd.fillRect(0, 0, w, h, 0x2104);
  drawGraphFrame(w, h, graph_x0, graph_y0, graph_w, graph_h);

  // ---TARGET_VALUEの赤点線描画 ---
  int target_y = graph_y0 + graph_h - (int)(graph_h * TARGET_VALUE / 40.0f);
  for (int x = graph_x0; x < graph_x0 + graph_w; x += 8)
  {
    M5.Lcd.drawLine(x, target_y, x + 4, target_y, RED); // 4px線+4px空白
  }

  int n = data.size();
  if (n > 1)
  {
    for (int i = 1; i < n; ++i)
    {
      int x0 = graph_x0 + graph_w * (i - 1) / (n - 1);
      int y0 = graph_y0 + graph_h - (int)(graph_h * data[i - 1] / 40.0f);
      int x1 = graph_x0 + graph_w * i / (n - 1);
      int y1 = graph_y0 + graph_h - (int)(graph_h * data[i] / 40.0f);
      M5.Lcd.drawLine(x0, y0, x1, y1, GREEN);
      M5.Lcd.fillCircle(x1, y1, 3, GREEN);
    }
    M5.Lcd.fillCircle(graph_x0 + graph_w,
                      graph_y0 + graph_h - (int)(graph_h * data.back() / 40.0f), 6, RED);
  }
  else
  {
    M5.Lcd.setCursor(graph_x0 + 10, h / 2 - 10);
    M5.Lcd.println("Waiting for data...");
  }
  delay(20);
}
