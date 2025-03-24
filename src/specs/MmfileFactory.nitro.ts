import { type HybridObject } from 'react-native-nitro-modules'
import type { Mmfile, EncryptedMmfile } from './Mmfile.nitro'

export interface MmfileFactory extends HybridObject<{ ios: 'c++', android: 'c++' }> {
  openMmfile(path: string): Mmfile;
  openEncryptedMmfile(path: string, key: ArrayBuffer): EncryptedMmfile;
}
