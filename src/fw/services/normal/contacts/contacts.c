/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "contacts.h"

#include "kernel/pbl_malloc.h"
#include "services/normal/blob_db/contacts_db.h"
#include "services/normal/timeline/attributes_actions.h"
#include "system/logging.h"

static Contact* prv_deserialize_contact(SerializedContact *serialized_contact,
                                        const size_t serialized_contact_data_len) {
  if (serialized_contact_data_len == 0 || serialized_contact == NULL) {
    return NULL;
  }

  size_t string_alloc_size;
  uint8_t attributes_per_address[serialized_contact->num_addresses];
  bool r = attributes_address_parse_serial_data(serialized_contact->num_attributes,
                                                serialized_contact->num_addresses,
                                                serialized_contact->data,
                                                serialized_contact_data_len,
                                                &string_alloc_size,
                                                attributes_per_address);
  if (!r) {
    return NULL;
  }

  const size_t alloc_size = attributes_address_get_buffer_size(serialized_contact->num_attributes,
                                                               serialized_contact->num_addresses,
                                                               attributes_per_address,
                                                               string_alloc_size);

  Contact *contact = kernel_zalloc_check(sizeof(Contact) + alloc_size);

  uint8_t *buffer = (uint8_t *)contact + sizeof(Contact);
  uint8_t *const buf_end = buffer + alloc_size;

  attributes_address_init(&contact->attr_list,
                          &contact->addr_list,
                          &buffer,
                          serialized_contact->num_attributes,
                          serialized_contact->num_addresses,
                          attributes_per_address);

  if (!attributes_address_deserialize(&contact->attr_list,
                                      &contact->addr_list,
                                      buffer,
                                      buf_end,
                                      serialized_contact->data,
                                      serialized_contact_data_len)) {
    kernel_free(contact);
    return NULL;
  }

  contact->id = serialized_contact->uuid;
  contact->flags = serialized_contact->flags;
  return contact;
}

Contact* contacts_get_contact_by_uuid(const Uuid *uuid) {
  SerializedContact *serialized_contact = NULL;
  const int serialized_contact_data_len = contacts_db_get_serialized_contact(uuid,
                                                                             &serialized_contact);

  Contact *contact = prv_deserialize_contact(serialized_contact, serialized_contact_data_len);

  contacts_db_free_serialized_contact(serialized_contact);

  return contact;
}

void contacts_free_contact(Contact *contact) {
  kernel_free(contact);
}
