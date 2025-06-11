import { type HybridObject } from 'react-native-nitro-modules'
import type { Mmfile, EncryptedMmfile } from './Mmfile.nitro'

export interface ReadDirItem {
  mtime: number;          // The last modified date of the file (seconds since epoch)
  name: string;           // The name of the item
  path: string;           // The absolute path to the item
  size: number;           // Size in bytes
  isFile: boolean;        // Is the item just a file?
  isDirectory: boolean;   // Is the item a directory?
}

export interface MmfilePackage extends HybridObject<{ ios: 'c++', android: 'c++' }> {
  baseDirectory: string;

  openMmfile(path: string, readOnly?: boolean): Mmfile;
  openEncryptedMmfile(path: string, key: ArrayBuffer, readOnly?: boolean): EncryptedMmfile;

  fileExists(path: string): boolean;
  getFileSize(path: string): number;
  getEncryptedFileSize(path: string): number;

  readDir(path: string): Promise<ReadDirItem[]>
  readDirSync(path: string): ReadDirItem[]

  unlink(path: string): Promise<void>;
  unlinkSync(path: string): void;
}
