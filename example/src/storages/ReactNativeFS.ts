import {appendFile, unlink, DocumentDirectoryPath} from 'react-native-fs';

var path = DocumentDirectoryPath + '/test.txt';

export async function prepareReactNativeFS(): Promise<void> {
  try {
    await unlink(path);
  } catch (error) {
    console.log("Couldn't delete file: ", error);
  }
}

export async function appendReactNativeFS(buffer: ArrayBuffer, str: string): Promise<void> {
  return appendFile(path, str)
}
