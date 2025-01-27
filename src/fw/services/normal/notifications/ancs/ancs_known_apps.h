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

// @nolint
// please don't change these values manually, they are derived from the spreadsheet
// "Notification Colors"

#if PLATFORM_TINTIN
// Tintin does not have the color arg in its App Metadata. Remove it.
#define APP(id, icon, color) { id, icon }
#else
#define APP(id, icon, color) { id, icon, color }
#endif

    APP(IOS_CALENDAR_APP_ID, TIMELINE_RESOURCE_TIMELINE_CALENDAR, GColorRedARGB8),
    APP(IOS_FACETIME_APP_ID, TIMELINE_RESOURCE_NOTIFICATION_FACETIME, GColorIslamicGreenARGB8),
    APP(IOS_MAIL_APP_ID, TIMELINE_RESOURCE_GENERIC_EMAIL, GColorVividCeruleanARGB8),
    APP(IOS_PHONE_APP_ID, TIMELINE_RESOURCE_TIMELINE_MISSED_CALL, GColorPictonBlueARGB8),
    APP(IOS_REMINDERS_APP_ID, TIMELINE_RESOURCE_NOTIFICATION_REMINDER, GColorFollyARGB8),
    APP(IOS_SMS_APP_ID, TIMELINE_RESOURCE_GENERIC_SMS, GColorIslamicGreenARGB8),
    APP("com.atebits.Tweetie2", TIMELINE_RESOURCE_NOTIFICATION_TWITTER, GColorVividCeruleanARGB8),
    APP("com.burbn.instagram", TIMELINE_RESOURCE_NOTIFICATION_INSTAGRAM, GColorCobaltBlueARGB8),
    APP("com.facebook.Facebook", TIMELINE_RESOURCE_NOTIFICATION_FACEBOOK, GColorCobaltBlueARGB8),
    APP("com.facebook.Messenger", TIMELINE_RESOURCE_NOTIFICATION_FACEBOOK_MESSENGER, GColorBlueMoonARGB8),
    APP("com.getpebble.pebbletime", TIMELINE_RESOURCE_NOTIFICATION_FLAG, GColorOrangeARGB8),
    APP("com.google.calendar", TIMELINE_RESOURCE_TIMELINE_CALENDAR, GColorVeryLightBlueARGB8),
    APP("com.google.Gmail", TIMELINE_RESOURCE_NOTIFICATION_GMAIL, GColorRedARGB8),
    APP("com.google.hangouts", TIMELINE_RESOURCE_NOTIFICATION_GOOGLE_HANGOUTS, GColorJaegerGreenARGB8),
    APP("com.google.inbox", TIMELINE_RESOURCE_NOTIFICATION_GOOGLE_INBOX, GColorBlueMoonARGB8),
    APP("com.microsoft.Office.Outlook", TIMELINE_RESOURCE_NOTIFICATION_OUTLOOK, GColorCobaltBlueARGB8),
    APP("com.orchestra.v2", TIMELINE_RESOURCE_NOTIFICATION_MAILBOX, GColorVividCeruleanARGB8),
    APP("com.skype.skype", TIMELINE_RESOURCE_NOTIFICATION_SKYPE, GColorVividCeruleanARGB8),
    APP("com.tapbots.Tweetbot3", TIMELINE_RESOURCE_NOTIFICATION_TWITTER, GColorVividCeruleanARGB8),
    APP("com.toyopagroup.picaboo", TIMELINE_RESOURCE_NOTIFICATION_SNAPCHAT, GColorIcterineARGB8),
    APP("com.yahoo.Aerogram", TIMELINE_RESOURCE_NOTIFICATION_YAHOO_MAIL, GColorIndigoARGB8),
    APP("jp.naver.line", TIMELINE_RESOURCE_NOTIFICATION_LINE, GColorIslamicGreenARGB8),
    APP("net.whatsapp.WhatsApp", TIMELINE_RESOURCE_NOTIFICATION_WHATSAPP, GColorIslamicGreenARGB8),
    APP("ph.telegra.Telegraph", TIMELINE_RESOURCE_NOTIFICATION_TELEGRAM, GColorVividCeruleanARGB8),
#if !PLATFORM_TINTIN
    APP("com.blackberry.bbm1", TIMELINE_RESOURCE_NOTIFICATION_BLACKBERRY_MESSENGER, GColorDarkGrayARGB8),
    APP("com.getpebble.pebbletime.enterprise", TIMELINE_RESOURCE_NOTIFICATION_FLAG, GColorOrangeARGB8),
    APP("com.google.GoogleMobile", TIMELINE_RESOURCE_NOTIFICATION_GENERIC, GColorBlueMoonARGB8),
    APP("com.google.ios.youtube", TIMELINE_RESOURCE_NOTIFICATION_GENERIC, GColorClearARGB8),
    APP("com.hipchat.ios", TIMELINE_RESOURCE_NOTIFICATION_HIPCHAT, GColorCobaltBlueARGB8),
    APP("com.iwilab.KakaoTalk", TIMELINE_RESOURCE_NOTIFICATION_KAKAOTALK, GColorYellowARGB8),
    APP("com.kik.chat", TIMELINE_RESOURCE_NOTIFICATION_KIK, GColorIslamicGreenARGB8),
    APP("com.tencent.xin", TIMELINE_RESOURCE_NOTIFICATION_WECHAT, GColorKellyGreenARGB8),
    APP("com.viber", TIMELINE_RESOURCE_NOTIFICATION_VIBER, GColorVividVioletARGB8),
    APP("com.amazon.Amazon", TIMELINE_RESOURCE_NOTIFICATION_AMAZON, GColorChromeYellowARGB8),
    APP("com.google.Maps", TIMELINE_RESOURCE_NOTIFICATION_GOOGLE_MAPS, GColorBlueMoonARGB8),
    APP("com.google.photos", TIMELINE_RESOURCE_NOTIFICATION_GOOGLE_PHOTOS, GColorBlueMoonARGB8),
    APP("com.apple.mobileslideshow", TIMELINE_RESOURCE_NOTIFICATION_IOS_PHOTOS, GColorBlueMoonARGB8),
    APP("com.linkedin.LinkedIn", TIMELINE_RESOURCE_NOTIFICATION_LINKEDIN, GColorCobaltBlueARGB8),
    APP("com.tinyspeck.chatlyio", TIMELINE_RESOURCE_NOTIFICATION_SLACK, GColorFollyARGB8),
#endif

#undef APP
