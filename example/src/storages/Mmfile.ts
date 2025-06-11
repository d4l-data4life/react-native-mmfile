import {openMmfile} from '@d4l/react-native-mmfile';

const path = 'k1';

let mmapFile = openMmfile(path)

export function prepareMmfile() {
  mmapFile.clear();
}

export function appendMmfile(buffer: ArrayBuffer) {
  mmapFile.append(buffer);
}
