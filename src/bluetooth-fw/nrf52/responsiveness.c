#include "bluetooth/responsiveness.h"
#include "bluetooth/gap_le_connect.h"

#include <inttypes.h>

bool bt_driver_le_connection_parameter_update(
    const BTDeviceInternal *addr, const BleConnectionParamsUpdateReq *req) {
  return true;
}
