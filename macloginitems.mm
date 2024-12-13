#include "macloginitems.h"
#include <Cocoa/Cocoa.h>

bool MacLoginItems::isAutorun(const char* appPath)
{
  NSString *nsAppPath=[NSString stringWithUTF8String:appPath];
  UInt32 seedValue;
  CFURLRef thePath;
  LSSharedFileListRef loginItems=LSSharedFileListCreate(NULL,kLSSharedFileListSessionLoginItems,NULL);
  CFArrayRef loginItemsArray=LSSharedFileListCopySnapshot(loginItems,&seedValue);
  for (id item in (NSArray *)loginItemsArray) {
    LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
    if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr){
      if ([[(NSURL *)thePath path] hasPrefix:nsAppPath]){
        CFRelease(thePath);
        CFRelease(loginItemsArray);
        CFRelease(loginItems);
        return true;
      }
      CFRelease(thePath);
    }
  }
  CFRelease(loginItemsArray);
  CFRelease(loginItems);
  return false;
}

void MacLoginItems::setAutorun(bool enable,const char* appPath)
{
  NSString *nsAppPath=[NSString stringWithUTF8String:appPath];
  if (enable){
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:nsAppPath];
    LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemLast, NULL, NULL, url, NULL, NULL);
    CFRelease(item);
    CFRelease(loginItems);
  }else{
    UInt32 seedValue;
    CFURLRef thePath;
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
    CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);
    for (id item in (NSArray *)loginItemsArray) {
      LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
      if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr){
        if ([[(NSURL *)thePath path] hasPrefix:nsAppPath]){
          LSSharedFileListItemRemove(loginItems, itemRef);
        }
        CFRelease(thePath);
      }
    }
    CFRelease(loginItemsArray);
    CFRelease(loginItems);
  }
}
