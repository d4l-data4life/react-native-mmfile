import { NitroModules } from 'react-native-nitro-modules'

import type { Mmfile, EncryptedMmfile } from './specs/Mmfile.nitro'
import type { MmfilePackage, ReadDirItem } from './specs/MmfilePackage.nitro'
import type { MmfilePlatformContext } from './specs/MmfilePlatformContext.nitro'

const MmfilePackageImpl = NitroModules.createHybridObject<MmfilePackage>('MmfilePackage')
const MmfilePlatformContextImpl = NitroModules.createHybridObject<MmfilePlatformContext>('MmfilePlatformContext')

const baseDirectory: string = MmfilePlatformContextImpl.getBaseDirectory();

function relativeToAbsolutePath(path: string): string {
    // If the path is absolute, return it directly
    if (path.startsWith('/')) {
        return path;
    }
    // Otherwise, prepend the base directory
    return baseDirectory + '/' + path;
}

export function getBaseDirectory(): string {
    return MmfilePlatformContextImpl.getBaseDirectory();
}

export function openMmfile(path: string): Mmfile {
    return MmfilePackageImpl.openMmfile(relativeToAbsolutePath(path));
}

export function openEncryptedMmfile(path: string, key: ArrayBuffer): EncryptedMmfile {
    return MmfilePackageImpl.openEncryptedMmfile(relativeToAbsolutePath(path), key);
}

export function readDir(path: string): Promise<ReadDirItem[]> {
    return MmfilePackageImpl.readDir(relativeToAbsolutePath(path));
}

export function unlink(path: string): Promise<void> {
    return MmfilePackageImpl.unlink(relativeToAbsolutePath(path));
}

export function fileExists(path: string): boolean {
    return MmfilePackageImpl.fileExists(relativeToAbsolutePath(path));
}

export function getFileSize(path: string): number {
    return MmfilePackageImpl.getFileSize(relativeToAbsolutePath(path));
}

export function getEncryptedFileSize(path: string): number {
    return MmfilePackageImpl.getEncryptedFileSize(relativeToAbsolutePath(path));
}
