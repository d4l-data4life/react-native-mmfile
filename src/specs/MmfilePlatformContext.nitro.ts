import {
  type HybridObject,
} from 'react-native-nitro-modules'

export interface MmfilePlatformContext extends HybridObject<{ ios: 'swift', android: 'kotlin' }> {
  /**
   * Gets the base directory of the documents storage
   */
  getBaseDirectory(): string;
  /**
   * Get the App Group directory if it exists, or `undefined` otherwise.
   *
   * The App Group directory will be used instead of a custom path to use the same
   * MMKV instance between the iOS app, Widgets, and other companion apps.
   *
   * To set an App Group, add a `AppGroup` field to `Info.plist`
   *
   * @platform ios
   */
  getAppGroupDirectory(): string | undefined;
}
