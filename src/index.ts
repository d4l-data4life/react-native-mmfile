import { NitroModules } from 'react-native-nitro-modules'

import type { MmfilePackage as MmfilePackageSpec } from './specs/MmfilePackage.nitro'
import type { MmfilePlatformContext as MmfilePlatformContextSpec } from './specs/MmfilePlatformContext.nitro'

const MmfilePackage = NitroModules.createHybridObject<MmfilePackageSpec>('MmfilePackage')
const MmfilePlatformContext = NitroModules.createHybridObject<MmfilePlatformContextSpec>('MmfilePlatformContext')

MmfilePackage.baseDirectory = MmfilePlatformContext.getBaseDirectory();

export default MmfilePackage;
