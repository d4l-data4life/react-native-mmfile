# API Reference

## Global Methods

```tsx
import { openMmfile, openEncryptedMmfile } from 'react-native-mmfile';

 // open a file
let mmapFile = openMmfile('file1');

// 128-bit example key
let key = new Uint8Array([
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff]);

// open a file with encryption
let encryptedMmapFile = openEncryptedMmfile('file2', key);
```

There are two functions to open a file, `openMmfile` and `openEncryptedMmfile`. The first one opens a file in read/write mode, while the second one opens a file with encryption. The encryption key is passed as an argument.

## File Objects

```tsx
export interface MmfileBase {
  readonly size: number
  readonly capacity: number
  readonly readOnly: boolean
  readonly filePath: string
  readonly isOpen: boolean

  close(): void
  resize(newSize: number): void
  clear(): void
  append(buffer: ArrayBuffer): void
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

## Clear a key

```tsx
function App() {
  const [username, setUsername] = useMMKVString("user.name")
  // ...
  const onLogout = useCallback(() => {
    setUsername(undefined)
  }, [])
}
```

## Objects

```tsx
type User = {
  id: string
  username: string
  age: number
}

function App() {
  const [user, setUser] = useMMKVObject<User>("user")
}
```

## Reactively use an MMKV Instance

```tsx
function App() {
  const storage = useMMKV()
  // ...
  const onLogin = useCallback((username) => {
    storage.set("user.name", "Marc")
  }, [storage])
}
```

## Reactively use individual keys from custom instances

```tsx
function App() {
  const globalStorage = useMMKV()
  const userStorage = useMMKV({ id: `${userId}.storage` })

  const [username, setUsername] = useMMKVString("user.name", userStorage)
}
```

## Listen to value changes

```tsx
function App() {
  useMMKVListener((key) => {
    console.log(`Value for "${key}" changed!`)
  })
}
```

## Listen to value changes on a specific instance

```tsx
function App() {
  const storage = useMMKV({ id: `${userId}.storage` })

  useMMKVListener((key) => {
    console.log(`Value for "${key}" changed in user storage!`)
  }, storage)
}
```
