import { type HybridObject } from 'react-native-nitro-modules'

export interface Mmfile extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  readonly size: number
  readonly capacity: number
  readonly readOnly: boolean
  readonly filePath: string

  resize(newSize: number): void
  clear(): void
  append(buffer: ArrayBuffer): void
  write(offset: number, buffer: ArrayBuffer): void
  read(offset: number, buffer: ArrayBuffer): number
}

export interface EncryptedMmfile extends Mmfile {
}
