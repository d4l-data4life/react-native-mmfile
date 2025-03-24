import { type HybridObject } from 'react-native-nitro-modules'

export interface Mmfile
  extends HybridObject<{ ios: 'swift'; android: 'kotlin' }> {
  readonly size: number

  resize(newSize: number): void
  clear(): void
  append(data: ArrayBuffer): void
  write(offset: number, data: ArrayBuffer): void
  read(offset: number, data: ArrayBuffer): number
}

export interface EncryptedMmfile extends Mmfile {
}
