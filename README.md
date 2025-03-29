# react-native-mmfile

This is a ReactNative module for blazingly fast file I/O with optional encryption.

Features:

- 🚀 High Performance: Leverages mmap for memory-mapped file I/O, significantly outperforming fs and react-native-fs.
Built on the new React Native architecture with [NitroModules](https://nitro.margelo.com), it enables zero-copy operations and direct ArrayBuffer access.
- 📱 Cross-Platform: Seamlessly works on both iOS and Android
- 🛠️ Simple API: Easily `read`, `write`, or `append` files using the `Mmfile` or `EncryptedMmfile` objects.
- 🔒 Optional Encryption: Supports AES-128 in CTR mode for real-time encryption/decryption. The cipher is implemented in C++ and optimized for performance using NEON (ARM) and AES-NI (Intel).
- 📦 Zero Dependencies: No external libraries required—pure, efficient native code.

## Installation

<h3>
  React Native  <a href="#"><img src="./docs/img/react-native.png" height="15" /></a>
</h3>

```sh
bun add react-native-mmfile react-native-nitro-modules
cd ios && pod install
```

<h3>
  Expo  <a href="#"><img src="./docs/img/expo.png" height="12" /></a>
</h3>

```sh
expo install react-native-quick-crypto
expo prebuild
```

## Usage

```ts
import { openMmfile, openEncryptedMmfile } from 'react-native-mmfile';

// encode 'Hello World' to ArrayBuffer using UTF-8 encoding
let data = new TextEncoder().encode('Hello World').buffer;

let mmapFile = openMmfile('test1.txt');
mmapFile.append(data);
mmapFile.close();
```

## Contributing

Contribute a change to this template to update it for newer React Native versions.

