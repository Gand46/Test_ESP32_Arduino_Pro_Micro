#include <Bluepad32.h>

static constexpr uint32_t UART_BAUD = 1000000;
static constexpr int UART_TX_PIN = 17;
static constexpr int UART_RX_PIN = 16;

static constexpr uint8_t PKT_MAGIC   = 0xA5;
static constexpr uint8_t PKT_VERSION = 0x01;
static constexpr uint32_t KEEPALIVE_MS = 8;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

struct __attribute__((packed)) BridgePacket {
  uint8_t magic;
  uint8_t version;
  uint8_t seq;
  uint8_t flags;      // bit0: controller connected

  uint8_t dpad;
  uint16_t buttons;
  uint8_t misc;

  uint16_t brake;     // 0..1023
  uint16_t throttle;  // 0..1023

  int16_t lx;         // Bluepad32 raw, typically around -511..512
  int16_t ly;
  int16_t rx;
  int16_t ry;

  uint8_t crc;
};

static BridgePacket lastSent = {};
static uint8_t txSeq = 0;
static uint32_t lastTxMs = 0;

static uint8_t crc8(const uint8_t* data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static inline int16_t clamp_i16(int v) {
  if (v > 32767) return 32767;
  if (v < -32768) return -32768;
  return (int16_t)v;
}

static inline uint16_t clamp_u10(int v) {
  if (v < 0) return 0;
  if (v > 1023) return 1023;
  return (uint16_t)v;
}

static ControllerPtr firstConnectedController() {
  for (int i = 0; i < BP32_MAX_GAMEPADS; ++i) {
    if (myControllers[i] != nullptr && myControllers[i]->isConnected()) {
      return myControllers[i];
    }
  }
  return nullptr;
}

static BridgePacket buildPacket(ControllerPtr ctl) {
  BridgePacket pkt = {};
  pkt.magic = PKT_MAGIC;
  pkt.version = PKT_VERSION;
  pkt.seq = txSeq++;
  pkt.flags = 0;

  if (ctl && ctl->isConnected()) {
    pkt.flags |= 0x01;
    pkt.dpad = (uint8_t)ctl->dpad();
    pkt.buttons = (uint16_t)ctl->buttons();
    pkt.misc = (uint8_t)ctl->miscButtons();
    pkt.brake = clamp_u10(ctl->brake());
    pkt.throttle = clamp_u10(ctl->throttle());
    pkt.lx = clamp_i16(ctl->axisX());
    pkt.ly = clamp_i16(ctl->axisY());
    pkt.rx = clamp_i16(ctl->axisRX());
    pkt.ry = clamp_i16(ctl->axisRY());
  }

  pkt.crc = crc8(reinterpret_cast<const uint8_t*>(&pkt), sizeof(pkt) - 1);
  return pkt;
}

static bool payloadEqualsIgnoringSeqCrc(const BridgePacket& a, const BridgePacket& b) {
  return a.magic == b.magic &&
         a.version == b.version &&
         a.flags == b.flags &&
         a.dpad == b.dpad &&
         a.buttons == b.buttons &&
         a.misc == b.misc &&
         a.brake == b.brake &&
         a.throttle == b.throttle &&
         a.lx == b.lx &&
         a.ly == b.ly &&
         a.rx == b.rx &&
         a.ry == b.ry;
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; ++i) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; ++i) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  for (auto& c : myControllers) {
    c = nullptr;
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  delay(300);
}

void loop() {
  const bool updated = BP32.update();
  (void)updated;

  ControllerPtr ctl = firstConnectedController();
  BridgePacket pkt = buildPacket(ctl);

  uint32_t now = millis();
  bool changed = !payloadEqualsIgnoringSeqCrc(pkt, lastSent);
  bool keepalive = (now - lastTxMs) >= KEEPALIVE_MS;

  if (changed || keepalive) {
    Serial2.write(reinterpret_cast<const uint8_t*>(&pkt), sizeof(pkt));
    lastSent = pkt;
    lastTxMs = now;
  }

  // Small pause to avoid a tight spin loop while staying responsive.
  delay(1);
}
