import { NitroModules } from 'react-native-nitro-modules'

import type { MmfileFactory } from './specs/MmfileFactory.nitro'
import type { MmfilePlatformContext } from './specs/MmfilePlatformContext.nitro'

export type MmfileFactoryType = MmfileFactory
export type MmfilePlatformContextType = MmfilePlatformContext
export const MmfileFactoryImpl = NitroModules.createHybridObject<MmfileFactory>('MmfileFactory')
export const MmfilePlatformContextImpl = NitroModules.createHybridObject<MmfilePlatformContext>('MmfilePlatformContext')
