#include "MyApp.h"

#define min   // évite le conflit NimBLE/min-max
#define max
#include <host/ble_hs.h>
#undef max
#undef min

#include <os/os_mbuf.h>
#include <cstring>

using namespace Pinetime::Applications::Screens;

/* ------------------------------------------------------------------ */
/*                       UUIDs et handles                              */
/* ------------------------------------------------------------------ */
namespace {

constexpr ble_uuid128_t MakeUuid(const uint8_t (&b)[16]) {
  return ble_uuid128_t{
    .u = {.type = BLE_UUID_TYPE_128},
    .value = { b[0],  b[1],  b[2],  b[3],  b[4],  b[5],  b[6],  b[7],
               b[8],  b[9],  b[10], b[11], b[12], b[13], b[14], b[15] }
  };
}

constexpr ble_uuid128_t kSrvUuid = MakeUuid(
  {0x00,0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11});
constexpr ble_uuid128_t kUrlUuid = MakeUuid(
  {0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,0xf6,0xe5,0xd4,0xc3,0xb2,0xa1});
constexpr ble_uuid128_t kRspUuid = MakeUuid(
  {0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19});

static uint16_t urlHandle{};
static uint16_t rspHandle{};

/* -------- Write callback : téléphone -> montre -------- */
int rspWriteCb(uint16_t /*conn*/, uint16_t /*attr*/,
               ble_gatt_access_ctxt* ctxt, void* arg) {

  auto* self = static_cast<MyApp*>(arg);
  if (!self) return BLE_ATT_ERR_UNLIKELY;

  const uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
  if (len >= 244) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

  char buf[244]{};
  os_mbuf_copydata(ctxt->om, 0, len, buf);
  self->OnHttpResponse(buf, len);
  return 0;
}

/* ----------------- GATT definitions ------------------ */
static ble_gatt_chr_def chrs[] = {
  { /* URL characteristic : Notify */
    .uuid        = &kUrlUuid.u,
    .access_cb   = nullptr,
    .arg         = nullptr,
    .descriptors = nullptr,
    .flags       = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
    .min_key_size= 0,
    .val_handle  = &urlHandle,
  },
  { /* Response characteristic : Write */
    .uuid        = &kRspUuid.u,
    .access_cb   = rspWriteCb,
    .arg         = nullptr,            // patché plus bas
    .descriptors = nullptr,
    .flags       = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
    .min_key_size= 0,
    .val_handle  = &rspHandle,
  },
  { 0 }   // terminator
};

static ble_gatt_svc_def svcs[] = {
  {
    .type            = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid            = &kSrvUuid.u,
    .includes        = nullptr,
    .characteristics = chrs,
  },
  { 0 }   // terminator
};

} // anonymous namespace


/* ------------------------------------------------------------------ */
/*                         Implémentation                              */
/* ------------------------------------------------------------------ */

MyApp* MyApp::Self = nullptr;

MyApp::MyApp() {
  Self = this;

  /* UI ---------------------------------------------------- */
  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text_static(title, "Loading…");

  /* BLE : enregistre le service --------------------------- */
  chrs[1].arg = this;               // passe 'this' au callback
  ble_gatts_count_cfg(svcs);
  ble_gatts_add_svcs(svcs);

  /* Requête de test -------------------------------------- */
  SendUrl("https://api.chucknorris.io/jokes/random");
}

MyApp::~MyApp() {
  lv_obj_clean(lv_scr_act());
  Self = nullptr;
}

void MyApp::SendUrl(std::string_view url) {
  os_mbuf* om = ble_hs_mbuf_from_flat(url.data(), url.size());
  ble_gattc_notify_custom(BLE_HS_CONN_HANDLE_NONE, urlHandle, om);
}

void MyApp::OnHttpResponse(const char* data, size_t /*len*/) {
  lv_label_set_text(title, data);   // affiche texte brut
}
