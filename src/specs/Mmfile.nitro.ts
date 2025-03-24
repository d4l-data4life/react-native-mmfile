import { type HybridObject } from 'react-native-nitro-modules'

export interface Mmfile
  extends HybridObject<{ ios: 'swift'; android: 'kotlin' }> {
  readonly size: number

  resize(newSize: number): void
  clear(): void
  append(buffer: ArrayBuffer): void
  write(offset: number, buffer: ArrayBuffer): void
  read(offset: number, buffer: ArrayBuffer): number
}

export interface EncryptedMmfile extends Mmfile {
}
