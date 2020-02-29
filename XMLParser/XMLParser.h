/*
    The purpose of this very simple XML parser is to be able to read .SVG 
    files, nothing more. 
    Not implemented are:
    - reading CDATA sections (it skips it) 
    - support for &lt; &gt; &amp; &apos; &quot;
    - namespaces
    - probably a whole lot more
    - keeping whitespace 100% as read (not needed for .SVG)
    - tag names can only use characters in the 'A' .. 'z' range
      for their first character

    Resources: 
    https://www.xml.com/pub/a/98/10/guide0.html
    https://www.w3.org/TR/xmlschema-0/

*/

/*
    Good resource: https://www.w3.org/TR/xmlschema-0/

    XML file structure example (.SVG:)

    <?xml version="1.0" encoding="iso-8859-1"?>
    <!-- Generator: Adobe Illustrator 19.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->
    <svg version="1.1" id="Capa_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
    viewBox="0 0 511.998 511.998" style="enable-background:new 0 0 511.998 511.998;" xml:space="preserve">
    <path style="fill:#FF98C5;" d="M323.669,263.214l-67.668-16.839l-59.247,14.999l-63.109,135.565l61.58,100.097
    c19.452,4.89,39.808,7.498,60.777,7.498c64.59,0,123.419-24.644,167.614-65.035v-72.565L323.669,263.214z"/>
    <path style="fill:#7CB777;" d="M310.527,13.483L168.648,76.895l54.441,76.812l53.04,46.353l90.545,46.057l126-66.135
    C465.99,96.837,396.718,32.778,310.527,13.483z"/>
    <path style="fill:#FADC60;" d="M276.13,200.061c0,0,106.414-35.158,216.545-20.078c7.69,23.961,11.859,49.501,11.859,76.017
    c0,72.672-31.195,138.052-80.919,183.497C332.362,364.687,256,246.374,256,246.374L276.13,200.061z"/>
    <path style="fill:#85E7FF;" d="M176.649,114.664l-38.005-40.77l-61.027,9.073C34.205,127.715,7.467,188.731,7.467,256
    c0,27.163,4.373,53.3,12.427,77.766l88.43-53.024l64.377-72.201l-12.186-44.309L176.649,114.664z"/>
    <path style="fill:#EB5E8A;" d="M296.005,474.529c-20.969,0-41.326-2.608-60.777-7.498l0,0c-29.556-7.429-57.007-20.154-81.29-37.102
    l41.283,67.106l0,0l0,0c19.452,4.89,39.808,7.498,60.777,7.498c64.59,0,123.419-24.644,167.613-65.035l0,0v-0.213
    C386.311,461.654,342.667,474.529,296.005,474.529z"/>
    <path style="fill:#B6A4FF;" d="M172.7,208.543l24.053,52.83c0,0-25.89,96.171-1.529,235.661
    c-82.682-20.783-148.901-82.985-175.329-163.268C87.184,246.863,172.7,208.543,172.7,208.543z"/>
    <path style="fill:#FD6F71;" d="M256,7.467c18.733,0,36.979,2.087,54.527,6.016c-61.35,66.827-87.439,140.225-87.439,140.225
    l-62.574,10.526c0,0-33.418-41.438-82.898-81.266C122.786,36.411,186.009,7.467,256,7.467z"/>
    <circle style="fill:#EFEDEE;" cx="212.768" cy="200.057" r="63.358"/>
    <path style="fill:#57D0E6;" d="M59.9,303.763c-8.054-24.466-12.427-50.605-12.427-77.766c0-54.363,17.468-104.638,47.082-145.546
    l-16.937,2.518l0,0l0,0C34.205,127.715,7.467,188.731,7.467,256c0,27.163,4.373,53.3,12.427,77.766l41.716-25.013
    C61.024,307.098,60.45,305.436,59.9,303.763z"/>
    <path style="fill:#E34B4C;" d="M87.341,90.984c8.854-13.653,18.999-26.391,30.282-38.02l0,0c3.293-3.394,6.694-6.684,10.174-9.886
    c-0.002,0.001-0.003,0.002-0.005,0.003c-6.824,4.119-13.433,8.558-19.811,13.292c-0.181,0.134-0.36,0.27-0.541,0.405
    c-1.907,1.423-3.792,2.874-5.656,4.351c-0.2,0.159-0.402,0.314-0.602,0.474c-3.932,3.132-7.761,6.387-11.493,9.747
    c-0.46,0.415-0.916,0.835-1.373,1.253c-1.377,1.259-2.738,2.534-4.086,3.824c-0.49,0.469-0.984,0.935-1.47,1.409
    c-1.737,1.688-3.457,3.395-5.145,5.135l0,0l0,0C80.934,85.637,84.174,88.314,87.341,90.984z"/>
    <path style="fill:#937FE6;" d="M92.671,443.31c29.348,25.619,64.765,44.226,102.553,53.725c-2.835-16.234-4.982-31.872-6.576-46.849
    C128.101,421.139,81.144,368.295,59.9,303.763c-1.231-3.741-2.366-7.525-3.422-11.343c-12.382,12.323-24.763,26.081-36.583,41.346
    c3.303,10.035,7.229,19.789,11.731,29.213C46.213,393.523,67.17,421.048,92.671,443.31z"/>
    <path d="M12.848,336.144c4.489,13.674,10.127,26.969,16.871,39.686c1.94,3.66,6.481,5.054,10.141,3.112
    c3.659-1.941,5.054-6.481,3.112-10.141c-5.722-10.789-10.636-22.055-14.635-33.541c0.938-1.187,1.879-2.372,2.83-3.549
    c13.751-16.999,28.835-32.929,45.026-47.621c16.999-15.466,35.234-29.392,54.177-42.383c0.945-0.632,18.43-11.202,18.256-11.571
    c0.057,0.121,0.122,0.238,0.18,0.359c1.424,2.992,3.063,5.876,4.896,8.637c6.835,10.315,16.294,18.738,27.42,24.325
    c2.198,1.104,4.468,2.117,6.769,2.987c-3.767,17.169-6.103,34.626-7.788,52.112c-1.851,19.301-2.719,38.562-2.757,57.953
    c-0.071,36.547,2.804,73.12,8.435,109.228c0.051,0.326,0.097,0.647,0.149,0.974c-51.362-15.589-96.278-48.109-127.382-92.481
    c-2.378-3.392-7.056-4.215-10.448-1.836c-3.392,2.378-4.215,7.056-1.836,10.448c35.406,50.506,87.669,86.556,147.164,101.511
    c82.719,20.793,172.328-1.74,235.279-59.275c25.995-23.758,46.48-52.019,60.884-83.995c25.806-57.289,29.454-123.536,10.257-183.349
    C472.226,91.662,400.322,25.937,312.199,6.209C259.711-5.542,203.941-0.429,154.559,20.92C61.653,61.083,0,154.875,0,256.046
    C0,283.204,4.379,310.347,12.848,336.144z M168.296,166.554c7.651-10.025,18.628-17.549,30.985-20.639
    c0.05-0.012,0.1-0.022,0.15-0.035c32.354-7.928,64.93,15.156,68.848,48.036c3.593,30.155-20.016,58.864-50.085,61.785
    c-28.893,2.807-56.632-18.949-60.691-47.747C155.446,193.364,159.35,178.276,168.296,166.554z M256.034,497.078
    c-18.283,0-36.496-2.062-54.21-6.133c-0.82-4.902-1.591-9.81-2.306-14.728c-7.455-51.23-9.365-103.434-4.512-155.005
    c1.594-16.958,3.703-34.145,7.418-50.86c0.01-0.049,0.02-0.094,0.031-0.142c0.005,0.221,6.387,0.639,7.121,0.673
    c11.665,0.533,23.401-1.848,33.923-6.916c3.798-1.829,7.445-3.991,10.894-6.481c9.484,14.03,19.373,27.791,29.534,41.339
    c23.676,31.567,48.789,62.116,75.893,90.81c15.983,16.92,32.661,33.206,50.329,48.368c0.327,0.281,1.648,1.414,1.983,1.698
    C368.573,476.766,313.482,497.078,256.034,497.078z M481.668,171.14c-0.667-0.07-30.21-2.354-44.332-2.354
    c-2.52-0.011-5.049-0.008-7.588,0.021c-4.086,0.046-7.462,3.508-7.416,7.586c0.046,4.09,3.489,7.48,7.585,7.415
    c2.593-0.029,5.175-0.035,7.746-0.021c16.476,0.084,32.957,1.047,49.311,3.078c6.699,22.386,10.092,45.634,10.092,69.181
    c0,34.455-7.116,67.722-21.151,98.878c-12.669,28.123-30.335,53.195-52.564,74.613c-0.816-0.688-1.63-1.378-2.444-2.073
    c-18.406-15.711-35.545-32.762-52.224-50.278c-0.024-0.025-0.048-0.05-0.071-0.075c-22.842-24.366-44.094-50.137-64.547-76.527
    c-11.731-15.171-23.247-30.612-33.886-46.571c-0.369-0.538-4.808-6.417-4.554-6.701c3.25-3.643,6.063-7.624,8.536-11.83
    c5.217-9.063,8.437-19.335,9.276-29.763c0.06-0.018,0.123-0.037,0.185-0.055c2.159-0.634,4.324-1.251,6.493-1.849
    c15.87-4.394,32.167-8.45,48.456-10.956c10.366-1.803,20.742-3.486,31.182-4.812c10.165-1.31,20.375-2.307,30.599-3.024
    c4.06-0.284,7.243-3.943,6.958-8.007c-0.285-4.068-3.941-7.245-8.007-6.958c-11.402,0.799-22.85,1.822-34.17,3.439
    c-8.466,1.106-16.909,2.393-25.32,3.855c-18.942,3.293-38.094,7.561-56.743,12.827c-0.032,0.009-0.068,0.02-0.1,0.029
    c-1.349-9.545-4.681-18.788-9.705-27.014c-7.623-12.478-19.037-22.518-32.917-28.376c-0.311-0.131-0.619-0.269-0.932-0.396
    c0.031-0.067,0.06-0.131,0.092-0.199c9.998-21.544,21.658-42.304,34.804-62.083c10.102-15.199,21.096-29.817,32.999-43.653
    c1.926-2.238,3.875-4.456,5.849-6.652C390.343,40.605,453.739,96.98,481.668,171.14z M160.511,34.69
    c30.201-13.056,62.34-19.676,95.523-19.676c13.472,0,26.94,1.124,40.19,3.349c-2.344,2.719-4.645,5.476-6.928,8.246
    c-10.994,13.341-21.136,27.38-30.529,41.89c-11.782,18.181-21.827,37.142-31.503,56.493c-0.828,1.754-1.645,3.513-2.449,5.279
    c-0.171-0.03-0.344-0.049-0.515-0.077c-18.929-3.078-38.674,1.749-54.02,13.273c-3.547,2.664-6.871,5.652-9.892,8.973
    c-9.445-10.939-19.361-21.468-29.588-31.677c-13.358-13.336-27.311-26.123-41.832-38.186c-0.047-0.039-0.093-0.078-0.14-0.117
    C109.756,62.273,133.835,46.222,160.511,34.69z M78.315,93.239c0.127,0.105,0.254,0.21,0.381,0.315
    c20.332,16.839,39.495,35.116,57.479,54.439c5.137,5.519,10.205,11.109,15.076,16.865c0.017,0.02,0.032,0.038,0.049,0.058
    c-3.74,6.514-6.388,13.538-7.892,20.847c-0.008,0.041-0.018,0.08-0.026,0.121c-1.959,9.566-1.893,19.62,0.165,29.167
    c0.042,0.194,0.071,0.391,0.114,0.584c-0.154,0.091-0.316,0.188-0.471,0.28c-21.39,12.619-41.719,27.094-60.827,42.952
    c-21.364,17.73-41.202,37.329-58.973,58.666c-0.111,0.133-0.223,0.262-0.352,0.417c-5.337-20.129-8.036-40.889-8.036-61.904
    C15.002,195.349,37.433,137.806,78.315,93.239z"/>
    </svg>
*/

#pragma once
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>

constexpr auto TAG_START_MARKER				= "<";
constexpr auto TAG_END_MARKER				= ">";
constexpr auto TAG_CLOSE_MARKER				= "/";
constexpr auto XML_VERSION_START_MARKER		= "<?";
constexpr auto XML_VERSION_END_MARKER		= "?>";
constexpr auto XML_SELF_CLOSING_MARKER		= "/>";
constexpr auto COMMENT_TAG_MARKER			= "!";
constexpr auto ATTRIBUTE_ASSIGNMENT_MARKER	= "=";
constexpr auto ATTRIBUTE_VALUE_DELIMITER1	= "'";
constexpr auto ATTRIBUTE_VALUE_DELIMITER2	= "\"";
constexpr auto COMMENT_START_MARKER			= "<!--";
constexpr auto COMMENT_END_MARKER			= "-->";
constexpr auto COMMENT_ILLEGAL_CONTENT		= "--";
constexpr auto CDATA_START_MARKER			= "<![CDATA[";
constexpr auto CDATA_END_MARKER				= "]]>";

class XMLAttribute {
public:
    std::string                 name;
    std::string                 value;
};

class XMLElement {
public:
    std::string                 tag;
    std::string                 value;  // change to std::vector< std::string > ?
    std::vector< std::string >  cdata;  // remove?
    std::vector< XMLAttribute > attributes;
    std::vector< XMLElement >   children;
};

class XMLFile {
public:
    XMLFile( std::string filename ) :
        filename_( filename ) 
    { 
        isLoaded_ = readFile(); 
    }
    bool    isLoaded() { return isLoaded_; }
    void    print();
    void    printXMLElement( const XMLElement& xmlElement,int depth );

private:
	std::string& trimTrailingSpaces(std::string& trimStr);
	// reads the .xml textfile into a stream of strings, returns false on error
    bool    readFileToBuffer( 
        std::stringstream& rawData,
        std::string& filename );

    /* 
        This function reads anything between 'startMarker' and 'endMarker' 
        from the stream and puts the contents, if found, in destStr. The 
        opening ( e.g. '<') marker and closing marker (e.g. '>') are included.
        The string preceding the first tag found are stored in precedingStr.
        This string may contain spaces (so more than one word).
        The string trailingStr contains any character that immediately follow  
        the tag closing marker '>'. This string never contains spaces.
        Note that on starting this function:
        - rawData is modified (we read from it)
        - tagStr is cleared initially
        - trailingStr is cleared initially
        - precedingStr is NOT cleared, we start looking for a new tag in this
        string, where we left off with the previous tag search. If not empty,
        it SHOULD end with a space
    */
    bool    extractFromStringStream(
        std::stringstream& rawData,
        std::string startMarker,
        std::string endMarker,
        std::string& destStr,
        std::string& precedingStr,
        std::string& trailingStr );

    /* 
      returns true if the element is a comment. The bool isValidComment is
      set to false if there is a -- string in the comment (which is not 
      allowed).
    */
    bool    tagIsAComment( const std::string& tagStr,bool& isValidComment );
    bool    tagIsSelfClosing( const std::string& tagStr );
    bool    tagIsAClosingTag( const std::string& tagStr );
    bool    tagIsXMLPrologTag( const std::string& tagStr );
    bool    tagIsXMLVersionTag( const std::string& tagStr );
    bool    extractTagNameAndAttributes( 
        std::string& tagStr,
        XMLElement& xmlElement );

    // returns true on success, false on error
    bool    readFile();

private:
    bool                        isLoaded_;
    std::string                 filename_;
    std::string                 xmlVersion_;
    std::string                 xmlCharset_;
    XMLElement                  elements_;
};


