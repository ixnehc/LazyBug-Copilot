using System;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.VisualStudio.Shell;
using Task = System.Threading.Tasks.Task;
using Microsoft.VisualStudio.Text.Tagging;
using System.ComponentModel.Composition;
using System.Windows.Media;
using Microsoft.VisualStudio.Text.Classification; // For EditorFormatDefinition
using Microsoft.VisualStudio.Text.Editor;         // For MarkerFormatDefinition
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.Text;
using System.Collections.Generic;


namespace LazyBugPlugInCSharp
{
    /// <summary>
    /// Represents the tag used to mark lines for background highlighting.
    /// It references the name of the MarkerFormatDefinition.
    /// </summary>
    internal class LazyBugDiffHilightTag : TextMarkerTag
    {
        public const string FormatDefinitionName = "MarkerFormatDefinition/LazyBugDiffHilight";

        public LazyBugDiffHilightTag() : base(FormatDefinitionName)
        {
        }
    }

    /// <summary>
    /// Defines the visual format for the line background highlight.
    /// This is exported via MEF so the editor can discover it.
    /// </summary>
    [Export(typeof(EditorFormatDefinition))]
    [Name(LazyBugDiffHilightTag.FormatDefinitionName)] // Must match the name used in LineBackgroundTag
    [UserVisible(true)] // Makes this format visible in Tools > Options > Environment > Fonts and Colors
    internal class LazyBugDiffHilightFormatDefinition : MarkerFormatDefinition
    {
        public LazyBugDiffHilightFormatDefinition()
        {
            // The name displayed in the "Fonts and Colors" dialog
            this.DisplayName = "My Custom Line Highlight";

            // Set the background color for the marked line
            this.BackgroundColor = Colors.LightYellow;

            // Optionally, set the foreground color (text color)
            // this.ForegroundColor = Colors.DarkBlue;

            // ZOrder determines the drawing order. Higher values are drawn on top.
            this.ZOrder = 5;
        }
    }


    /// <summary>
    /// Tagger responsible for finding lines that should be highlighted
    /// and creating LineBackgroundTag instances for them.
    /// </summary>
    internal class LazyBugDiffHilightTagger : ITagger<LazyBugDiffHilightTag> // Note: Specific tag type here
    {
        private readonly ITextView _textView;
        private readonly ITextBuffer _buffer;

        public LazyBugDiffHilightTagger(ITextView textView, ITextBuffer buffer)
        {
            _textView = textView;
            _buffer = buffer;

            // Subscribe to buffer changes to re-evaluate tags when text changes
            _buffer.Changed += OnBufferChanged;
            // Optionally, subscribe to layout changes if your logic depends on view layout
            // _textView.LayoutChanged += OnLayoutChanged;
        }

        /// <summary>
        /// This event is raised when tags need to be re-evaluated by the editor.
        /// </summary>
        public event EventHandler<SnapshotSpanEventArgs> TagsChanged;

        /// <summary>
        /// Called by the editor to get tags for the specified spans.
        /// </summary>
        public IEnumerable<ITagSpan<LazyBugDiffHilightTag>> GetTags(NormalizedSnapshotSpanCollection spans)
        {
            if (spans.Count == 0) // Nothing to do if no spans are provided
            {
                yield break;
            }

            ITextSnapshot currentSnapshot = _buffer.CurrentSnapshot;

            foreach (SnapshotSpan spanToProcess in spans) // Each 'spanToProcess' is a single SnapshotSpan
            {
                // Get the line number of the start of the span
                int firstLineNumber = currentSnapshot.GetLineNumberFromPosition(spanToProcess.Start);

                // Get the line number of the end of the span.
                // If the span is empty, end position is the same as start.
                // If the span has length, the last character is at spanToProcess.End - 1.
                int lastLineNumber;
                if (spanToProcess.Length == 0)
                {
                    lastLineNumber = firstLineNumber;
                }
                else
                {
                    // Get the line containing the last character of the span
                    lastLineNumber = currentSnapshot.GetLineNumberFromPosition(spanToProcess.End - 1);
                }

                // Iterate through all lines from firstLineNumber to lastLineNumber inclusive
                for (int i = firstLineNumber; i <= lastLineNumber; i++)
                {
                    ITextSnapshotLine line = currentSnapshot.GetLineFromLineNumber(i);

                    // *** This is where your custom logic goes to decide if a line should be highlighted ***
                    if (ShouldHighlightLine(line)) // Assuming ShouldHighlightLine takes ITextSnapshotLine
                    {
                        // Create a new TagSpan for the entire line
                        // The line.Extent gives a SnapshotSpan covering the whole line, excluding line break characters.
                        // If you want to include line break characters, use line.ExtentIncludingLineBreak.
                        var tagSpan = new TagSpan<LazyBugDiffHilightTag>(line.Extent, new LazyBugDiffHilightTag());
                        yield return tagSpan;
                    }
                }
            }
        }

        /// <summary>
        /// Custom logic to determine if a given line should be highlighted.
        /// </summary>
        /// <param name="line">The text snapshot line to evaluate.</param>
        /// <returns>True if the line should be highlighted, false otherwise.</returns>
        private bool ShouldHighlightLine(ITextSnapshotLine line)
        {
            string lineText = line.GetText();

            // Example: Highlight lines containing the word "IMPORTANT"
            if (lineText.Contains("IMPORTANT"))
            {
                return true;
            }

            if (line.LineNumber == 4)
            {
               return true;
            }

            // Example: Highlight every other line
            // if (line.LineNumber % 2 == 0)
            // {
            //     return true;
            // }

            return false; // Default: do not highlight
        }

        /// <summary>
        /// Handles text buffer changes to trigger a re-tagging.
        /// </summary>
        private void OnBufferChanged(object sender, TextContentChangedEventArgs e)
        {
            // If the edit spans multiple lines, or if lines were added/removed,
            // it's often easiest to re-tag the affected area.
            foreach (var change in e.Changes)
            {
                // Create a span that covers the changed area in the new snapshot
                var changedSpan = new SnapshotSpan(e.After, change.NewSpan);
                TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(changedSpan));
            }
        }

        // Optional: If your highlighting depends on view layout (e.g., visible lines)
        // private void OnLayoutChanged(object sender, TextViewLayoutChangedEventArgs e)
        // {
        //    // Raise TagsChanged for all translated spans that have changed
        //    foreach (var span in e.NewOrReformattedSpans)
        //    {
        //        TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(span));
        //    }
        // }
    }

    /// <summary>
    /// MEF component that creates LineBackgroundTagger instances for text views.
    /// </summary>
    [Export(typeof(IViewTaggerProvider))]
    [ContentType("text")] // Apply to all text-based content types. You can be more specific (e.g., "CSharp", "XML").
    [TagType(typeof(TextMarkerTag))] // Specifies that this provider creates TextMarkerTag instances.
    internal class LineBackgroundTaggerProvider : IViewTaggerProvider
    {
        public ITagger<T> CreateTagger<T>(ITextView textView, ITextBuffer buffer) where T : ITag
        {
            // We only want to create a tagger for the view's primary buffer.
            // This prevents creating taggers for things like intellisense popups or other adornments.
            if (textView.TextBuffer != buffer)
            {
                return null;
            }

            // Create and return our custom tagger, ensuring it's castable to ITagger<T>
            // We use a Func<ITagger<T>> to allow lazy instantiation or to pass dependencies if needed.
            // For this simple case, direct instantiation is fine.
            return new LazyBugDiffHilightTagger(textView, buffer) as ITagger<T>;
        }
    }

}
