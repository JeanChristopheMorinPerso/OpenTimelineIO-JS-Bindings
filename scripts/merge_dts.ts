import fs = require('fs')
import ts = require("typescript")

/** True if this is visible outside this file, false otherwise */
function isNodeExported(node: ts.Node): boolean {
    return (
        (ts.getCombinedModifierFlags(node as ts.Declaration) & ts.ModifierFlags.Export) !== 0
        // (node.parent && node.parent.kind === ts.SyntaxKind.SourceFile)
    )
}

const preJSSourceFile = ts.createSourceFile(
    './x.ts',   // fileName (value is not important)
    fs.readFileSync('./src/pre.d.ts', 'utf8'), // sourceText
    ts.ScriptTarget.Latest, // languageVersion
    true, // setParentNodes
)

let otioSourceFile = ts.createSourceFile(
    './x.ts',   // fileName (value is not important)
    fs.readFileSync('./install/opentimelineio.d.ts_bck', 'utf8'), // sourceText
    ts.ScriptTarget.Latest, // languageVersion
    true, // setParentNodes
)

/**
 * Get the JSDoc attached to a given node.
 */
function getJSDoc(node: ts.Node): string {
    const text = node.getFullText()
    const commentRanges = ts.getLeadingCommentRanges(text, 0)

    if (!commentRanges || commentRanges.length === 0) {
        return ''
    }

    // JS compiler only considers the last comment significant.
    const { pos, end } = commentRanges[commentRanges.length - 1]
    const comment = text.substring(pos, end).trim().replace(/\*\/$|^\/?\*/, "")
    const splittedComment = comment.split('\n')

    if (splittedComment[0] === '*') {
        splittedComment.shift()
    }

    const commentLines: string[] = []
    for (let line of splittedComment) {
        line = line.trim()

        if (line.match(/\s*\*$/)) {
            commentLines.push('')
            continue
        }

        line = line.trim().replace(/\*\/$|^\/?\*|\/\//, "").trim()

        if (line) {
            commentLines.push(line)
        }
    }

    if (commentLines.length > 1) {
        const jsDoc = ts.factory.createJSDocComment(commentLines.join('\n'), [])
        const printer = ts.createPrinter()

        return printer.printNode(ts.EmitHint.Unspecified, jsDoc, null).trim().replace(/\*\/$|^\/?\*/gm, "")
    } else {
        return `* ${commentLines[0]}`
    }
}

function getNodeFullName(node: ts.Node): string | undefined {
    let declarationName = ts.getNameOfDeclaration(node as ts.Declaration)
    if (declarationName) {
        let name = declarationName.getText()

        let parentDeclarationName = ts.getNameOfDeclaration(node.parent as ts.Declaration)
        if (parentDeclarationName) {
            name = `${parentDeclarationName.getText()}.${name}`
        }
        return name
    }

    return undefined
}

const jsDocs = new Map<string, string>()

const preJSTransformer: ts.TransformerFactory<ts.Node> = context => {
    return sourceFile => {
        const visitor = (node: ts.Node): ts.Node => {
            const jsDoc = getJSDoc(node)
            const kind = ts.SyntaxKind[node.kind]
            if (jsDoc && !ts.isToken(node)) {
                const name = getNodeFullName(node)
                jsDocs.set(name, jsDoc)
                ts.setSyntheticLeadingComments(node, [{ kind: ts.SyntaxKind.MultiLineCommentTrivia, text: jsDoc, pos: -1, end: -1, hasTrailingNewLine: true }])
            }
            return ts.visitEachChild(node, visitor, context)
        }

        return ts.visitNode(sourceFile, visitor)
    }
}

let preJSTransformResult = ts.transform(preJSSourceFile, [preJSTransformer])

// Get exported declarations
const exportedNodes: ts.Statement[] = []
preJSTransformResult.transformed[0].forEachChild(child => {
    if (isNodeExported(child)) {
        // const jsDoc = getJSDoc(child)
        // if (jsDoc) {
        //     // Attach the JS doc as a leading comment.
        //     child = ts.addSyntheticLeadingComment(child, ts.SyntaxKind.MultiLineCommentTrivia, jsDoc, true)
        // }

        // TODO: Use a transformer to attach JSDocs of interface members.
        exportedNodes.push(child as ts.Statement)
    }
})

// console.log(ts.getSyntheticLeadingComments(otioSourceFile))
// Merge pre-js exported declarations into otio declarations.
otioSourceFile = ts.factory.updateSourceFile(otioSourceFile, [...exportedNodes, ...otioSourceFile.statements])

// console.log(getJSDoc(otioSourceFile))

// otioSourceFile = ts.addSyntheticLeadingComment(
//     otioSourceFile,
//     ts.SyntaxKind.MultiLineCommentTrivia,
//     getJSDoc(otioSourceFile),
//     true
// )

// Merge sidecar declarations into tsembind's CustomEmbindModule generated interface.
const transformer: ts.TransformerFactory<ts.Node> = context => {
    return sourceFile => {
        const visitor = (node: ts.Node): ts.Node => {
            if (ts.isInterfaceDeclaration(node)) {
                if (node.name.escapedText === 'CustomEmbindModule') {

                    const methods: ts.MethodSignature[] = []
                    for (const preJSNode of exportedNodes) {
                        const methodDeclaration = ts.factory.createMethodSignature(
                            [],
                            (preJSNode as ts.FunctionDeclaration).name,
                            undefined,
                            undefined,
                            (preJSNode as ts.FunctionDeclaration).parameters,
                            (preJSNode as ts.FunctionDeclaration).type
                        )

                        const fullName = getNodeFullName(preJSNode)
                        const jsDoc = jsDocs.get(fullName)
                        if (jsDoc) {
                            ts.setSyntheticLeadingComments(methodDeclaration, [{ kind: ts.SyntaxKind.MultiLineCommentTrivia, text: jsDoc, pos: -1, end: -1, hasTrailingNewLine: true }])
                        }

                        methods.push(methodDeclaration)
                    }

                    // The merge happens here.
                    return ts.factory.updateInterfaceDeclaration(
                        node,
                        node.modifiers,
                        node.name,
                        node.typeParameters,
                        node.heritageClauses,
                        [...methods, ...node.members],
                    )
                }
            }

            return ts.visitEachChild(node, visitor, context)
        }

        return ts.visitNode(sourceFile, visitor)
    }
}

let transformResult = ts.transform(otioSourceFile, [transformer])


// Render the final declaration file.
const printer = ts.createPrinter({ newLine: ts.NewLineKind.LineFeed, removeComments: false })
let resultFile = ts.createSourceFile("opentimelineio.d.ts", "", ts.ScriptTarget.Latest, /*setParentNodes*/ false, ts.ScriptKind.TS)
const result = printer.printNode(ts.EmitHint.Unspecified, transformResult.transformed[0], resultFile)
console.log(`// This file was generated by ./scripts/merge_dts.ts.

${result}`)
