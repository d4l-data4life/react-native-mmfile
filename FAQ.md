# How to release a new version

1. Increase the version in the package.json file
2. Add the version to [CHANGELOG.md](CHANGELOG.md)
3. Run `npm run specs`
4. Run `npm run build`
5. Run `cd example && npm run preinstall && cd ios && pod install`
6. Commit the changes
7. Push the changes to the repository
8. On GitHub, create a new release with a tag matching the version and the CHANGELOG entry as release notes

Publishing the release to npm is then done by the [publish workflow](.github/workflows/publish.yaml).
It can also be started manually via _Run workflow_ on the workflow page.
