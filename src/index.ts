import { NitroModules } from 'react-native-nitro-modules'

import type { MmfilePlatformContext } from './specs/MmfilePlatformContext.nitro'
import type { Mmfile, EncryptedMmfile } from './specs/Mmfile.nitro'

export const MmfileImpl = NitroModules.createHybridObject<Mmfile>('Mmfile')
export const EncryptedMmfileImpl = NitroModules.createHybridObject<EncryptedMmfile>('EncryptedMmfile')
export const MmfilePlatformContextImpl = NitroModules.createHybridObject<MmfilePlatformContext>('MmfilePlatformContext')
