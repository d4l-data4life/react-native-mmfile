# API Reference

## Global Methods

```tsx
openMmfile(path: string, readOnly?: boolean): Mmfile;
openEncryptedMmfile(path: string, key: ArrayBuffer, readOnly?: boolean): EncryptedMmfile;
```

There are two functions to open a file, `openMmfile()` and `openEncryptedMmfile()`.
The first one opens a regular file, while the second one opens a file with encryption.

- **path**: The argument is the path to the file to be opened, if it starts with a slash `/`, it is considered an absolute path, otherwise it is considered a relative path and stored in the app's directory. All parent folders are created if they do not exist.
- **key**: The encryption key is passed as an argument and must be a 128-bit key (16 bytes) stored in an `ArrayBuffer`.
- **readOnly**: The optional argument allows to open an existing file read-only, attempts to modify it, will throw an error.

Both functions either open an existing file or create a new one if it does not exist and the `readOnly` argument is not set to `true`. On success, they return a `Mmfile` or `EncryptedMmfile` object, respectively. If the file cannot be opened, an error is thrown.

### Example

```tsx
import { openMmfile, openEncryptedMmfile } from 'react-native-mmfile';

 // open a file
let mmapFile = openMmfile('file1');

// 128-bit example key
let key = new Uint8Array([
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff]);

// open a file with encryption
let encryptedMmapFile = openEncryptedMmfile('file2', key.buffer);
```

## Mmfile and EncryptedMmfile

The objects returned by the `openMmfile()` and `openEncryptedMmfile()` can be used to read and write to the file. They have the following methods:

```tsx
export interface MmfileBase {
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
```

### Basic Functions

- **append()**: Appends a buffer to the end of the file. The file is resized to fit the new data.
- **write()**: Writes a buffer to the file at the specified offset. The file is resized to fit the new data.
- **read()**: Reads data from the file at the specified offset into the provided buffer. The buffer size determines how much data is read, so it must be resized accordingly beforehand. The return value is the actual number of bytes read (which may be less than the buffer size if the end of the file is reached).
- **resize()**: Resizes the file to the specified size.
- **size()**: Returns the size of the file.
- **clear()**: Clears the file, setting its size to 0.

To improve performance and reduce the number of system calls and remappings, the file is resized with extra headroom to the next multiple of 64kB, especially when writing to the file. The `strictResize` option allows you to control this behavior. If set to `true`, the file will be resized to the exact size specified, which may result in a performance hit.
**capacity()** returns the headroom size until no further resizing is necesssary.

### Additional Functions

- **open**: Open a file. It takes the same arguments as the `openMmfile()` / `openEncryptedMmfile` functions, but reuses the same object.
- **close**: Close the file.
- **filePath**: Returns the absolute path to the currently open file, or an empty string if no file is open.
- **isOpen**: Returns `true` if the file is open, `false` otherwise.

If the object is garbage collected, the file is automatically closed, but it is recommended to close it as soon as no more operations are needed, so that it can be resized to its final size.
Encrypted files also store and update their length in the file header, so that no data is corrupted even if the app crashes before the file is closed.

## Encrypted File Format

Encrypted files begin with a custom header that contains the following fields in little-endian order:

```cpp
struct Header {
    uint16_t magic;   // 0xDA7A
    uint16_t version; // 0x0001
    uint64_t size;
    uint8_t  encryptedDataKey[16];
    uint8_t  iv[16];
    uint16_t keyHash;
    uint8_t  reserved[14];
};
```

- **magic** identifies the file as an encrypted file.
- **version** identifies the version of the file format.
- **size** stores the actual data size as returned by `size()`
- **encryptedDataKey** stores the data key encrypted by the provided key. The data key is used to encrypt and decrypt the data in the file and randomly generated at file creation time.
- **iv** stores the initialization vector for AES-CTR encryption. It also randomly generated at file creation time.
- **keyHash** stores a hash of the data key and IV to detect if the file is re-opened with the wrong key.
- **reserved** is reserved for future use.
