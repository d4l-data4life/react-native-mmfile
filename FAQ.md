# How to release a new version

1. Increase the version in the package.json file
2. Run `npm run specs`
3. Run `npm run build`
4. Run `cd example && npm run preinstall && cd ios && pod install`
5. Commit the changes
6. Push the changes to the repository
7. On GitHub, create a new release with a tag matching the version
8. Publish the release to npm with `npm publish`
