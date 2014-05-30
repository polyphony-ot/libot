# OT

## Operations

An operation is defined as a list of components that _must_ span the entire document.

An operation also has a `clientId`, a `hash`, and a `parent`. `clientId` is a unique integer value assigned to the client by the server. `hash` is a SHA1 hash of the documents contents after the operation is applied. `parent` is the SHA1 hash of the parent document that the operation is based on.

	{
		"clientId": n,
		"parent": "0b9cda76238570e1d5790452620b1df31dedf75d",
        "hash": "2346ad27d7568ba9896f1b7da6b5991251debdf2",
		"components": []
	}

## Components

### Skip

Skips _n_ number of characters from the current position.

Skips mean that the user did not modify the document at the skipped positions.

	{
		"type": "skip",
		"count": n
	}

### Insert

Inserts a string at the current position.

Inserts are guaranteed to be contiguous when merging. For example, say user A inserts "abc" at position 0 and user B inserts "def" at position 0. The document will converge on either "abcdef" or "defabc", never "adbecf" or "daebfc".

	{
		"type": "insert",
		"text": "str"
	}

### Delete

Deletes _n_ number of characters at the current position.

As with inserts, deletes are guaranteed to be contiguous when merging.

	{
		"type": "delete",
		"count": n
	}

### Open Element

Opens an element at the current position.

Elements are distinct from formatting boundaries in a few ways. The primary differences is that elements cannot overlap. For example, `<elemA><elemB></elemA></elemB>` is **not** valid.

The second difference is that elements can have a length of 0. A common example of this is a line break. The element `<newLine />` or `<newline></newline>` is valid and will not span any characters in the document. However, having a formatting boundary start and end at the same position is not valid.

	{
		"type": "openElement",
		"element": "element"
	}

### Close Element

Closes the last opened element at the current position.

	{
		"type": "closeElement"
	}

### Formatting Boundary

Inserts a formatting boundary at the current position.

Formatting boundaries make it easy to apply styling that can overlap - something that is not possible with elements. For example, say the user wants the text "**This _is some** text_". `<strong>This <em>is some</strong> text!</em>` would not be possible with elements. While possible to reorder the elements as `<strong>This <em>is some</em></strong><em> text!</em>`, it is much more confusing and difficult. Instead, you could use formatting boundaries to achieve the same effect.

	{
		"type": "formattingBoundary",
		"startFormatting": [
			{
				"name": "formattingName",
				"value": "formattingValue"
			}
		],
		"endFormatting": [
			{
				"name": "formattingName",
				"value": "formattingValue"
			}
		]
	}
