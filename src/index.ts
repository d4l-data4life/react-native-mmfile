import { NitroModules } from 'react-native-nitro-modules'

import type { Mmfile, EncryptedMmfile } from './specs/Mmfile.nitro'
import type { MmfileFactory } from './specs/MmfileFactory.nitro'
import type { MmfilePlatformContext } from './specs/MmfilePlatformContext.nitro'

const MmfileFactoryImpl = NitroModules.createHybridObject<MmfileFactory>('MmfileFactory')
const MmfilePlatformContextImpl = NitroModules.createHybridObject<MmfilePlatformContext>('MmfilePlatformContext')

const baseDirectory: string = MmfilePlatformContextImpl.getBaseDirectory();

export function openMmfile(path: string): Mmfile {
    // If the path is absolute, open the file directly
    if (path.startsWith('/')) {
        return MmfileFactoryImpl.openMmfile(path);
    } else {
        return MmfileFactoryImpl.openMmfile(baseDirectory + '/' + path);
    }
}

export function openEncryptedMmfile(path: string, key: ArrayBuffer): EncryptedMmfile {
    // If the path is absolute, open the file directly
    if (path.startsWith('/')) {
        return MmfileFactoryImpl.openEncryptedMmfile(path, key);
    } else {
        return MmfileFactoryImpl.openEncryptedMmfile(baseDirectory + '/' + path, key);
    }
}
