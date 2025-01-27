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



// ----------------------------------------------------------------
// Sample captured at: 2015-11-11 04:41:00 local, 2015-11-11 12:41:00 GMT
// NOTE: For some reason, the health10 branch thought there was an additional 2 to 3
// hours of sleep on the evening of 11/11/15 - probably from 8:30pm to 11:30pm. Not sure
// why it doesn't show up that way in the unit tests. Perhaps it depends on how much data
// is fed into the algorithm at once? Might want to try truncating the end of this data
// at various places to try and reproduce the error. 
AlgDlsMinuteData *activity_sample_2015_11_11_04_41_00(int *len) {
  // The unit tests parse the //> TEST_.* lines below for test values
  //> TEST_NAME pbl_29288
  //> TEST_VERSION 2
  //> TEST_TOTAL 0
  //> TEST_TOTAL_MIN 0
  //> TEST_TOTAL_MAX 0
  //> TEST_DEEP -1
  //> TEST_DEEP_MIN -1
  //> TEST_DEEP_MAX -1
  //> TEST_START_AT -1
  //> TEST_START_AT_MIN -1
  //> TEST_START_AT_MAX -1
  //> TEST_END_AT -1
  //> TEST_END_AT_MIN -1
  //> TEST_END_AT_MAX -1
  //> TEST_CUR_STATE_ELAPSED -1
  //> TEST_CUR_STATE_ELAPSED_MIN -1
  //> TEST_CUR_STATE_ELAPSED_MAX -1
  //> TEST_IN_SLEEP 0
  //> TEST_IN_SLEEP_MIN 0
  //> TEST_IN_SLEEP_MAX 0
  //> TEST_IN_DEEP_SLEEP 0
  //> TEST_IN_DEEP_SLEEP_MIN 0
  //> TEST_IN_DEEP_SLEEP_MAX 0
  //> TEST_WEIGHT 1.0

  // list of: {steps, orientation, vmc, light}
  static AlgDlsMinuteData samples[] = {
    // 0: Local time: 07:42:00 PM
    { 0, 0x73, 3, 161},
    { 0, 0x73, 55, 161},
    { 0, 0x73, 19, 158},
    { 0, 0x76, 517, 153},
    { 0, 0x75, 626, 152},
    { 0, 0x74, 92, 151},
    { 0, 0x74, 894, 152},
    { 0, 0x74, 12, 153},
    { 0, 0x74, 94, 154},
    { 0, 0x73, 1436, 152},
    { 0, 0x75, 365, 151},
    { 0, 0x75, 1123, 153},
    { 0, 0x74, 0, 153},
    { 0, 0x75, 608, 153},
    { 0, 0x74, 3, 153},
    // 15: Local time: 07:57:00 PM
    { 0, 0x74, 41, 154},
    { 0, 0x57, 573, 153},
    { 0, 0x75, 720, 152},
    { 0, 0x76, 1373, 153},
    { 0, 0x74, 0, 153},
    { 0, 0x75, 316, 154},
    { 0, 0x76, 468, 154},
    { 0, 0x48, 131, 154},
    { 21, 0x52, 4716, 154},
    { 0, 0x63, 1294, 153},
    { 0, 0x64, 2124, 148},
    { 0, 0x43, 3236, 152},
    { 0, 0x75, 801, 150},
    { 0, 0x73, 115, 153},
    { 0, 0x83, 0, 153},
    // 30: Local time: 08:12:00 PM
    { 0, 0x83, 0, 153},
    { 0, 0x82, 1, 153},
    { 0, 0x82, 43, 153},
    { 0, 0x82, 0, 153},
    { 0, 0x72, 432, 152},
    { 8, 0x73, 1410, 153},
    { 0, 0x73, 516, 158},
    { 0, 0x41, 3838, 150},
    { 12, 0x32, 4333, 152},
    { 36, 0x42, 8837, 159},
    { 5, 0x54, 3374, 162},
    { 0, 0x66, 1609, 165},
    { 0, 0x64, 922, 160},
    { 0, 0x45, 2036, 165},
    { 0, 0x66, 339, 162},
    // 45: Local time: 08:27:00 PM
    { 0, 0x74, 131, 161},
    { 0, 0x65, 510, 162},
    { 0, 0x74, 0, 163},
    { 0, 0x58, 265, 166},
    { 0, 0x59, 61, 162},
    { 0, 0x75, 314, 161},
    { 0, 0x75, 274, 161},
    { 0, 0x74, 1401, 164},
    { 0, 0x8e, 0, 164},
    { 0, 0x8e, 0, 164},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 164},
    // 60: Local time: 08:42:00 PM
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 164},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    // 75: Local time: 08:57:00 PM
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    // 90: Local time: 09:12:00 PM
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 189},
    { 0, 0x8f, 0, 191},
    { 0, 0x8f, 0, 191},
    { 0, 0x8f, 0, 191},
    { 0, 0x8f, 17, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    // 105: Local time: 09:27:00 PM
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 19, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    // 120: Local time: 09:42:00 PM
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 163},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    // 135: Local time: 09:58:00 PM
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    // 150: Local time: 10:13:00 PM
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    { 0, 0x8f, 0, 161},
    // 165: Local time: 10:28:00 PM
    { 0, 0x8d, 342, 159},
    { 0, 0x8f, 0, 162},
    { 0, 0x8f, 0, 154},
    { 0, 0x8f, 0, 154},
    { 0, 0x8f, 0, 157},
    { 0, 0x8f, 0, 159},
    { 0, 0x8f, 0, 155},
    { 0, 0x8f, 0, 159},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 166},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 167},
    // 180: Local time: 10:43:00 PM
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 167},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 166},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 169},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 166},
    // 195: Local time: 10:58:00 PM
    { 0, 0x8f, 0, 173},
    { 0, 0x8f, 0, 173},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 167},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 168},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 169},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 167},
    // 210: Local time: 11:13:00 PM
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 167},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 0, 0x8f, 0, 170},
    { 15, 0x61, 4139, 145},
    { 33, 0x42, 1522, 166},
    { 0, 0x42, 0, 166},
    { 0, 0x42, 2386, 160},
    { 0, 0x33, 2605, 161},
    { 0, 0x34, 5, 158},
    // 225: Local time: 11:28:01 PM
    { 0, 0x34, 0, 151},
    { 0, 0x34, 8, 156},
    { 16, 0x42, 4604, 141},
    { 12, 0x38, 4267, 141},
    { 0, 0x69, 0, 143},
    { 0, 0x69, 0, 141},
    { 0, 0x6a, 1184, 144},
    { 0, 0x6a, 0, 144},
    { 0, 0x6a, 0, 144},
    { 0, 0x6a, 0, 139},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 115, 142},
    { 0, 0x6a, 0, 139},
    // 240: Local time: 11:43:00 PM
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 0, 143},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 0, 136},
    { 0, 0x6a, 0, 142},
    { 0, 0x6a, 843, 142},
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 143},
    { 0, 0x69, 0, 139},
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 139},
    // 255: Local time: 11:58:00 PM
    { 0, 0x69, 0, 142},
    { 0, 0x69, 0, 145},
  };
  *len = ARRAY_LENGTH(samples);
  return samples;
}

