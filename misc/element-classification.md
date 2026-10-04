# Optional element classification

An `.elmt` is an XML `definition` containing translated `names`, drawing
primitives in `description`, and optional `elementInformations`. Classification
uses that existing information context; no new file version is required.

```xml
<elementInformations>
  <elementInformation name="standards" show="0">IEC60617;RGIE</elementInformation>
  <elementInformation name="countries" show="0">BE</elementInformation>
  <elementInformation name="tags" show="0">differential;RCD;protection;domestic</elementInformation>
  <elementInformation name="categories" show="0">electrical protection;residential</elementInformation>
</elementInformations>
```

All four fields are optional, independently. Values are separated by semicolons;
whitespace around entries and empty entries are ignored, and duplicate entries
are removed case-insensitively from the classification view. Original values
remain in `DiagramContext` and are saved with the existing XML writer. Fields
are free-form labels: `standards` describes references, not verified certification;
`countries` can hold country codes; `tags` describes uses; `categories` provides
classification labels independently of physical folders. Semicolons cannot be
part of an individual label. `show="0"` keeps these fields hidden by default.

`ElementsLocation::elementInformations()` reads collection definitions through
the pugixml overload of `DiagramContext::fromXml`. QDom handles project and editor
XML, including `ElementData::fromXml` for the editor. Both readers already accept
these names without a schema change.
`ElementClassification` exposes the lists and builds the search index used by
both `FileElementCollectionItem` and `XmlProjectElementCollectionItem`.

Collection search uses `QAbstractItemModel::match` on `Qt::UserRole + 1`, with
case-insensitive substring matching and recursive traversal.
`ElementsCollectionWidget::rankedSearch` ranks those hits and deduplicates them
by collection path; the picker uses the same method. Names, existing information
and unknown custom fields remain searchable. Queries need at least two characters
so `BE` can be searched. The existing `+` syntax combines terms as alternatives.
Try `RCD`, `IEC60617`, `RGIE`, `BE`, or `domestic` to find the same definition.

Definitions without classification keep the previous index text and do not gain
default fields on save. No collection is moved and no virtual category UI is
introduced. A complete illustrative definition is in
`tests/qttest/fixtures/classified_rcd.elmt`; its drawing is a test illustration,
not a certified IEC symbol.

Build with `PACKAGE_TESTS=ON`, then run:

```sh
cmake --build build --config Debug --target tst_elementclassification
ctest --test-dir build -C Debug -R '^tst_elementclassification$' --output-on-failure
```
