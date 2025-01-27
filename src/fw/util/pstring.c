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

#include <string.h>

#include "system/logging.h"

#include "kernel/pbl_malloc.h"
#include "util/pstring.h"

PascalString16 *pstring_create_pstring16(uint16_t size) {
  PascalString16* pstring = task_malloc_check(sizeof(uint16_t) + sizeof(char) * size);
  pstring->str_length = 0;
  return pstring;
}

PascalString16 *pstring_create_pstring16_from_string(char string[]) {
  uint16_t length = strlen(string);
  PascalString16* pstring;
  if (length == 0) {
    // Empty string
    pstring = task_malloc_check(sizeof(uint16_t) + sizeof(char) * 1);
    pstring->str_length = length;
    pstring->str_value[0] = '\0';
  } else {
    pstring = task_malloc_check(sizeof(uint16_t) + sizeof(char) * length);
    pstring->str_length = length;
    strncpy(pstring->str_value, string, length);
  }
  return pstring;
}

void pstring_destroy_pstring16(PascalString16 *pstring) {
  task_free(pstring);
}

void pstring_pstring16_to_string(const PascalString16 *pstring, char *string_out) {
  strncpy(string_out, pstring->str_value, pstring->str_length);
  string_out[(pstring->str_length)] = '\0';
}

void pstring_string_to_pstring16(char string[], PascalString16 *pstring_out) {
  uint16_t length = strlen(string);
  if (length == 0) {
    // Empty string
    pstring_out->str_length = length;
    pstring_out->str_value[0] = '\0';
  } else {
    pstring_out->str_length = length;
    strncpy(pstring_out->str_value, string, length);
  }
}

bool pstring_equal(const PascalString16 *ps1, const PascalString16 *ps2) {
  return ps1 && ps2 && (ps1->str_length == ps2->str_length) &&
         (memcmp(ps1->str_value, ps2->str_value, ps1->str_length) == 0);
}

bool pstring_equal_cstring(const PascalString16 *pstr, const char *cstr) {
  return pstr && cstr && (pstr->str_length == strlen(cstr)) &&
         (memcmp(pstr->str_value, cstr, pstr->str_length) == 0);
}

//-------

SerializedArray *pstring_create_serialized_array(uint16_t data_size) {
  size_t num = data_size + sizeof(uint16_t);
  size_t size = sizeof(uint8_t);
  SerializedArray *serialized_array = task_calloc_check(num, size);
  serialized_array->data_size = data_size;
  return serialized_array;
}

void pstring_destroy_serialized_array(SerializedArray* serialized_array) {
  task_free(serialized_array);
}

// Assumes a list of 0s is an empty list, not a list full of empty pstrings
uint16_t pstring_get_number_of_pstring16s_in_list(PascalString16List *pstring16_list) {
  size_t size = sizeof(uint16_t);
  uint16_t count = 0;
  uint16_t empty_count = 0;

  // Traverse list
  uint8_t *data_ptr = (pstring16_list->pstrings)->data;
  uint16_t pstring_length;
  do {
    pstring_length = (uint16_t) *data_ptr;
    if (pstring_length == 0) {
      empty_count++;
    } else {
      count += empty_count + 1;
      empty_count = 0;
    }
    data_ptr += pstring_length + size;
  } while ((&((pstring16_list->pstrings)->data[(pstring16_list->pstrings)->data_size]) - data_ptr) >
           1); // Need at least 2 bytes for another pstring

  if (count != 0) {
    count += empty_count;
  }

  return count;
}

void pstring_project_list_on_serialized_array(PascalString16List *pstring16_list,
                                              SerializedArray *serialized_array) {
  pstring16_list->pstrings = serialized_array;
  pstring16_list->count = pstring_get_number_of_pstring16s_in_list(pstring16_list);
}

bool pstring_add_pstring16_to_list(PascalString16List *pstring16_list, PascalString16* pstring) {
  size_t size = sizeof(uint16_t);

  // Traverse list
  uint8_t *data_ptr = (pstring16_list->pstrings)->data;
  uint8_t idx = 0;
  uint16_t pstring_length;
  do {
    if (idx == pstring16_list->count) {
      // End of list, copy contents of pstring
      memcpy(data_ptr, &(pstring->str_length), size);
      data_ptr += size;
      memcpy(data_ptr, pstring->str_value, pstring->str_length);
      (pstring16_list->count)++;
      return true;
    }
    // Advance pointer and index
    pstring_length = (uint16_t) *data_ptr;
    data_ptr += pstring_length + size;
    idx++;
  } while ((&((pstring16_list->pstrings)->data[(pstring16_list->pstrings)->data_size]) - data_ptr) >
           1); // Need at least 2 bytes for another pstring

  return false;
}

PascalString16 *pstring_get_pstring16_from_list(PascalString16List *pstring16_list,
                                                uint16_t index) {
  size_t size = sizeof(uint16_t);
  PascalString16 *pstring = NULL;
  uint16_t idx = 0;

  if (index >= pstring16_list->count) {
    return NULL;
  }

  // Traverse list
  uint8_t *data_ptr = (pstring16_list->pstrings)->data;
  uint16_t pstring_length;
  do {
    if (idx == index) {
      // Found the requested pstring
      pstring = (PascalString16*) data_ptr;
      break;
    }
    pstring_length = (uint16_t) *data_ptr;
    data_ptr += pstring_length + size;
    idx++;
  } while ((&((pstring16_list->pstrings)->data[(pstring16_list->pstrings)->data_size]) - data_ptr) >
           1); // Need at least 2 bytes for another pstring

  return pstring;
}

void pstring_print_pstring(PascalString16 *pstring) {
  PBL_LOG(LOG_LEVEL_DEBUG, "Length: %i ", pstring->str_length);
  char *buffer = task_malloc_check(sizeof(char) * (pstring->str_length + 1));
  pstring_pstring16_to_string(pstring, buffer);
  PBL_LOG(LOG_LEVEL_DEBUG, "%s", buffer);
  task_free(buffer);
  buffer = NULL;
}

void pstring_print_pstring16list(PascalString16List *list) {
  PBL_LOG(LOG_LEVEL_DEBUG, "Data size: %i ", (list->pstrings)->data_size);
  PascalString16 *pstring;
  for (int i = 0; i < list->count; i++) {
    pstring = pstring_get_pstring16_from_list(list, i);
    pstring_print_pstring(pstring);
  }
}
