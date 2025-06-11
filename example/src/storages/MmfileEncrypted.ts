import {openEncryptedMmfile} from '@d4l/react-native-mmfile';

const path = 'k3';
const aeskey = new Uint8Array([
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff]);

let mmapEncryptedFile = openEncryptedMmfile(path, aeskey.buffer)

export function prepareMmfileEncrypted() {
  mmapEncryptedFile.clear();
}

export function appendMmfileEncrypted(buffer: ArrayBuffer) {
  mmapEncryptedFile.append(buffer);
}
