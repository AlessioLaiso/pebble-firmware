#include <bluetooth/hrm_service.h>

bool bt_driver_is_hrm_service_supported(void) {
  return false;
}

void bt_driver_hrm_service_handle_measurement(const BleHrmServiceMeasurement *measurement,
                                              const BTDeviceInternal *permitted_devices,
                                              size_t num_permitted_devices) {
}
