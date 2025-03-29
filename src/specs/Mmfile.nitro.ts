import { type HybridObject } from 'react-native-nitro-modules'

export interface MmfileBase extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  readonly size: number
  readonly capacity: number
  readonly readOnly: boolean
  readonly filePath: string
  readonly isOpen: boolean

  close(): void
  resize(newSize: number, strictResize?: boolean): void
  clear(): void
  append(buffer: ArrayBuffer, strictResize?: boolean): void
  write(offset: number, buffer: ArrayBuffer): void
  read(offset: number, buffer: ArrayBuffer): number
}

export interface Mmfile extends MmfileBase {
  open(path: string, readOnly?: boolean): void
}

export interface EncryptedMmfile extends MmfileBase {
  open(path: string, key: ArrayBuffer, readOnly?: boolean): void
}
