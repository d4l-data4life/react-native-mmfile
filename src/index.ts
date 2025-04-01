import { NitroModules } from 'react-native-nitro-modules'

import type { MmfilePackage } from './specs/MmfilePackage.nitro'
import type { MmfilePlatformContext } from './specs/MmfilePlatformContext.nitro'

const MmfilePlatformContextImpl = NitroModules.createHybridObject<MmfilePlatformContext>('MmfilePlatformContext')
const Mmfile = NitroModules.createHybridObject<MmfilePackage>('MmfilePackage')

Mmfile.baseDirectory = MmfilePlatformContextImpl.getBaseDirectory();

export { Mmfile };
