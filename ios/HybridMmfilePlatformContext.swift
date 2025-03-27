import Foundation

class HybridMmfilePlatformContext : HybridMmfilePlatformContextSpec {
  func getBaseDirectory() -> String {
      #if os(tvOS)
      let paths = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask)
      #else
      let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
      #endif
      
      guard let documentPath = paths.first else {
          fatalError("Cannot find base-path to store Mmfile files!")
      }
      return documentPath.appendingPathComponent("rtnmmfile").path
  }

  func getAppGroupDirectory() -> String? {
        guard let appGroup = Bundle.main.object(forInfoDictionaryKey: "AppGroup") as? String else {
            return nil // No AppGroup set in Info.plist
        }
        
        guard let groupDir = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: appGroup) else {
            fatalError("An AppGroup was set in Info.plist, but it is not accessible via NSFileManager! AppGroup: \(appGroup)")
        }
        
        return groupDir.path
    }
}
