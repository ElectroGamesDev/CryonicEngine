#pragma once
#include "ThirdParty/imgui/imgui.h"
#include <cmath>

namespace RaylibWrapper
{
    constexpr int MAX_MATERIAL_MAPS = 12; // Defined "12" in raylib's config.h
    constexpr int MAX_MESH_VERTEX_BUFFERS = 9;

    // Vector2, 2 components
    typedef struct Vector2 {
        float x;                // Vector x component
        float y;                // Vector y component
    } Vector2;

    // Vector3, 3 components
    typedef struct Vector3 {
        float x;                // Vector x component
        float y;                // Vector y component
        float z;                // Vector z component
    } Vector3;

    // Vector4, 4 components
    typedef struct Vector4 {
        float x;                // Vector x component
        float y;                // Vector y component
        float z;                // Vector z component
        float w;                // Vector w component
    } Vector4;

    // Quaternion, 4 components (Vector4 alias)
    typedef Vector4 Quaternion;

    // Matrix, 4x4 components, column major, OpenGL style, right-handed
    typedef struct Matrix {
        float m0, m4, m8, m12;  // Matrix first row (4 components)
        float m1, m5, m9, m13;  // Matrix second row (4 components)
        float m2, m6, m10, m14; // Matrix third row (4 components)
        float m3, m7, m11, m15; // Matrix fourth row (4 components)
    } Matrix;

    // Color, 4 components, R8G8B8A8 (32bit)
    typedef struct Color {
        unsigned char r;        // Color red value
        unsigned char g;        // Color green value
        unsigned char b;        // Color blue value
        unsigned char a;        // Color alpha value
    } Color;

    // Rectangle, 4 components
    typedef struct Rectangle {
        float x;                // Rectangle top-left corner position x
        float y;                // Rectangle top-left corner position y
        float width;            // Rectangle width
        float height;           // Rectangle height
    } Rectangle;

    // Image, pixel data stored in CPU memory (RAM)
    typedef struct Image {
        void* data;             // Image raw data
        int width;              // Image base width
        int height;             // Image base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat type)
    } Image;

    // Texture, tex data stored in GPU memory (VRAM)
    typedef struct Texture {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat type)
    } Texture;

    // Texture2D, same as Texture
    typedef Texture Texture2D;

    // TextureCubemap, same as Texture
    typedef Texture TextureCubemap;

    // RenderTexture, fbo for texture rendering
    typedef struct RenderTexture {
        unsigned int id;        // OpenGL framebuffer object id
        Texture texture;        // Color buffer attachment texture
        Texture depth;          // Depth buffer attachment texture
    } RenderTexture;

    // RenderTexture2D, same as RenderTexture
    typedef RenderTexture RenderTexture2D;

    // NPatchInfo, n-patch layout info
    typedef struct NPatchInfo {
        Rectangle source;       // Texture source rectangle
        int left;               // Left border offset
        int top;                // Top border offset
        int right;              // Right border offset
        int bottom;             // Bottom border offset
        int layout;             // Layout of the n-patch: 3x3, 1x3 or 3x1
    } NPatchInfo;

    // GlyphInfo, font characters glyphs info
    typedef struct GlyphInfo {
        int value;              // Character value (Unicode)
        int offsetX;            // Character offset X when drawing
        int offsetY;            // Character offset Y when drawing
        int advanceX;           // Character advance position X
        Image image;            // Character image data
    } GlyphInfo;

    // Font, font texture and GlyphInfo array data
    typedef struct Font {
        int baseSize;           // Base size (default chars height)
        int glyphCount;         // Number of glyph characters
        int glyphPadding;       // Padding around the glyph characters
        Texture2D texture;      // Texture atlas containing the glyphs
        Rectangle* recs;        // Rectangles in texture for the glyphs
        GlyphInfo* glyphs;      // Glyphs info data
    } Font;

    // Camera, defines position/orientation in 3d space
    typedef struct Camera3D {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view aperture in Y (degrees) in perspective, used as near plane width in orthographic
        int projection;         // Camera projection: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
    } Camera3D;

    typedef Camera3D Camera;    // Camera type fallback, defaults to Camera3D

    // Camera2D, defines position/orientation in 2d space
    typedef struct Camera2D {
        Vector2 offset;         // Camera offset (displacement from target)
        Vector2 target;         // Camera target (rotation and zoom origin)
        float rotation;         // Camera rotation in degrees
        float zoom;             // Camera zoom (scaling), should be 1.0f by default
    } Camera2D;

    // Mesh, vertex data and vao/vbo
    typedef struct Mesh {
        int vertexCount;        // Number of vertices stored in arrays
        int triangleCount;      // Number of triangles stored (indexed or not)

        // Vertex attributes data
        float* vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
        float* texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
        float* texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
        float* normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
        float* tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
        unsigned char* colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
        unsigned short* indices;    // Vertex indices (in case vertex data comes indexed)

        // Animation vertex data
        float* animVertices;    // Animated vertex positions (after bones transformations)
        float* animNormals;     // Animated normals (after bones transformations)
        unsigned char* boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning) (shader-location = 6)
        float* boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)
        Matrix* boneMatrices;   // Bones animated transformation matrices
        int boneCount;          // Number of bones

        // OpenGL identifiers
        unsigned int vaoId;     // OpenGL Vertex Array Object id
        unsigned int* vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
    } Mesh;

    // Shader
    typedef struct Shader {
        unsigned int id;        // Shader program id
        int* locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
    } Shader;

    // MaterialMap
    typedef struct MaterialMap {
        Texture2D texture;      // Material map texture
        Color color;            // Material map color
        float value;            // Material map value
    } MaterialMap;

    // Material, includes shader and maps
    typedef struct Material {
        Shader shader;          // Material shader
        MaterialMap* maps;      // Material maps array (MAX_MATERIAL_MAPS)
        float params[4];        // Material generic parameters (if required)
    } Material;

    // Transform, vertex transformation data
    typedef struct Transform {
        Vector3 translation;    // Translation
        Quaternion rotation;    // Rotation
        Vector3 scale;          // Scale
    } Transform;

    // Bone, skeletal animation bone
    typedef struct BoneInfo {
        char name[32];          // Bone name
        int parent;             // Bone parent
    } BoneInfo;

    // Model, meshes, materials and animation data
    typedef struct Model {
        Matrix transform;       // Local transform matrix

        int meshCount;          // Number of meshes
        int materialCount;      // Number of materials
        Mesh* meshes;           // Meshes array
        Material* materials;    // Materials array
        int* meshMaterial;      // Mesh material number

        // Animation data
        int boneCount;          // Number of bones
        BoneInfo* bones;        // Bones information (skeleton)
        Transform* bindPose;    // Bones base transformation (pose)
    } Model;

    // ModelAnimation
    typedef struct ModelAnimation {
        int boneCount;          // Number of bones
        int frameCount;         // Number of animation frames
        BoneInfo* bones;        // Bones information (skeleton)
        Transform** framePoses; // Poses array by frame
    } ModelAnimation;

    // Ray, ray for raycasting
    typedef struct Ray {
        Vector3 position;       // Ray position (origin)
        Vector3 direction;      // Ray direction
    } Ray;

    // RayCollision, ray hit information
    typedef struct RayCollision {
        bool hit;               // Did the ray hit something?
        float distance;         // Distance to the nearest hit
        Vector3 point;          // Point of the nearest hit
        Vector3 normal;         // Surface normal of hit
    } RayCollision;

    // BoundingBox
    typedef struct BoundingBox {
        Vector3 min;            // Minimum vertex box-corner
        Vector3 max;            // Maximum vertex box-corner
    } BoundingBox;

    // Wave, audio wave data
    typedef struct Wave {
        unsigned int frameCount;    // Total number of frames (considering channels)
        unsigned int sampleRate;    // Frequency (samples per second)
        unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
        unsigned int channels;      // Number of channels (1-mono, 2-stereo, ...)
        void* data;                 // Buffer data pointer
    } Wave;

    // Opaque structs declaration
    // NOTE: Actual structs are defined internally in raudio module
    typedef struct rAudioBuffer rAudioBuffer;
    typedef struct rAudioProcessor rAudioProcessor;

    // AudioStream, custom audio stream
    typedef struct AudioStream {
        rAudioBuffer* buffer;       // Pointer to internal data used by the audio system
        rAudioProcessor* processor; // Pointer to internal data processor, useful for audio effects

        unsigned int sampleRate;    // Frequency (samples per second)
        unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
        unsigned int channels;      // Number of channels (1-mono, 2-stereo, ...)
    } AudioStream;

    // Sound
    typedef struct Sound {
        AudioStream stream;         // Audio stream
        unsigned int frameCount;    // Total number of frames (considering channels)
    } Sound;

    // Music, audio stream, anything longer than ~10 seconds should be streamed
    typedef struct Music {
        AudioStream stream;         // Audio stream
        unsigned int frameCount;    // Total number of frames (considering channels)
        bool looping;               // Music looping enable

        int ctxType;                // Type of music context (audio filetype)
        void* ctxData;              // Audio context data, depends on type
    } Music;

    // VrDeviceInfo, Head-Mounted-Display device parameters
    typedef struct VrDeviceInfo {
        int hResolution;                // Horizontal resolution in pixels
        int vResolution;                // Vertical resolution in pixels
        float hScreenSize;              // Horizontal size in meters
        float vScreenSize;              // Vertical size in meters
        float vScreenCenter;            // Screen center in meters
        float eyeToScreenDistance;      // Distance between eye and display in meters
        float lensSeparationDistance;   // Lens separation distance in meters
        float interpupillaryDistance;   // IPD (distance between pupils) in meters
        float lensDistortionValues[4];  // Lens distortion constant parameters
        float chromaAbCorrection[4];    // Chromatic aberration correction parameters
    } VrDeviceInfo;

    // VrStereoConfig, VR stereo rendering configuration for simulator
    typedef struct VrStereoConfig {
        Matrix projection[2];           // VR projection matrices (per eye)
        Matrix viewOffset[2];           // VR view offset matrices (per eye)
        float leftLensCenter[2];        // VR left lens center
        float rightLensCenter[2];       // VR right lens center
        float leftScreenCenter[2];      // VR left screen center
        float rightScreenCenter[2];     // VR right screen center
        float scale[2];                 // VR distortion scale
        float scaleIn[2];               // VR distortion scale in
    } VrStereoConfig;

    // File path list
    typedef struct FilePathList {
        unsigned int capacity;          // Filepaths max entries
        unsigned int count;             // Filepaths entries count
        char** paths;                   // Filepaths entries
    } FilePathList;

    // Camera system modes
    typedef enum {
        CAMERA_CUSTOM = 0,              // Custom camera
        CAMERA_FREE,                    // Free camera
        CAMERA_ORBITAL,                 // Orbital camera
        CAMERA_FIRST_PERSON,            // First person camera
        CAMERA_THIRD_PERSON             // Third person camera
    } CameraMode;

    // Camera projection
    typedef enum {
        CAMERA_PERSPECTIVE = 0,         // Perspective projection
        CAMERA_ORTHOGRAPHIC             // Orthographic projection
    } CameraProjection;

    // N-patch layout
    typedef enum {
        NPATCH_NINE_PATCH = 0,          // Npatch layout: 3x3 tiles
        NPATCH_THREE_PATCH_VERTICAL,    // Npatch layout: 1x3 tiles
        NPATCH_THREE_PATCH_HORIZONTAL   // Npatch layout: 3x1 tiles
    } NPatchLayout;

    enum WindowFlags {
        FLAG_VSYNC_HINT = 0x00000040,   // Set to try enabling V-Sync on GPU
        FLAG_FULLSCREEN_MODE = 0x00000002,   // Set to run program in fullscreen
        FLAG_WINDOW_RESIZABLE = 0x00000004,   // Set to allow resizable window
        FLAG_WINDOW_UNDECORATED = 0x00000008,   // Set to disable window decoration (frame and buttons)
        FLAG_WINDOW_HIDDEN = 0x00000080,   // Set to hide window
        FLAG_WINDOW_MINIMIZED = 0x00000200,   // Set to minimize window (iconify)
        FLAG_WINDOW_MAXIMIZED = 0x00000400,   // Set to maximize window (expanded to monitor)
        FLAG_WINDOW_UNFOCUSED = 0x00000800,   // Set to window non focused
        FLAG_WINDOW_TOPMOST = 0x00001000,   // Set to window always on top
        FLAG_WINDOW_ALWAYS_RUN = 0x00000100,   // Set to allow windows running while minimized
        FLAG_WINDOW_TRANSPARENT = 0x00000010,   // Set to allow transparent framebuffer
        FLAG_WINDOW_HIGHDPI = 0x00002000,   // Set to support HighDPI
        FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000, // Set to support mouse passthrough, only supported when FLAG_WINDOW_UNDECORATED
        FLAG_MSAA_4X_HINT = 0x00000020,   // Set to try enabling MSAA 4X
        FLAG_INTERLACED_HINT = 0x00010000    // Set to try enabling interlaced video format (for V3D)
    };

    // rlgl
    // Framebuffer attachment type
    // NOTE: By default up to 8 color channels defined, but it can be more
    typedef enum {
        RL_ATTACHMENT_COLOR_CHANNEL0 = 0,       // Framebuffer attachment type: color 0
        RL_ATTACHMENT_COLOR_CHANNEL1 = 1,       // Framebuffer attachment type: color 1
        RL_ATTACHMENT_COLOR_CHANNEL2 = 2,       // Framebuffer attachment type: color 2
        RL_ATTACHMENT_COLOR_CHANNEL3 = 3,       // Framebuffer attachment type: color 3
        RL_ATTACHMENT_COLOR_CHANNEL4 = 4,       // Framebuffer attachment type: color 4
        RL_ATTACHMENT_COLOR_CHANNEL5 = 5,       // Framebuffer attachment type: color 5
        RL_ATTACHMENT_COLOR_CHANNEL6 = 6,       // Framebuffer attachment type: color 6
        RL_ATTACHMENT_COLOR_CHANNEL7 = 7,       // Framebuffer attachment type: color 7
        RL_ATTACHMENT_DEPTH = 100,              // Framebuffer attachment type: depth
        RL_ATTACHMENT_STENCIL = 200,            // Framebuffer attachment type: stencil
    } rlFramebufferAttachType;

    // Framebuffer texture attachment type
    typedef enum {
        RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,   // Framebuffer texture attachment type: cubemap, +X side
        RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,   // Framebuffer texture attachment type: cubemap, -X side
        RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,   // Framebuffer texture attachment type: cubemap, +Y side
        RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,   // Framebuffer texture attachment type: cubemap, -Y side
        RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,   // Framebuffer texture attachment type: cubemap, +Z side
        RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,   // Framebuffer texture attachment type: cubemap, -Z side
        RL_ATTACHMENT_TEXTURE2D = 100,          // Framebuffer texture attachment type: texture2d
        RL_ATTACHMENT_RENDERBUFFER = 200,       // Framebuffer texture attachment type: renderbuffer
    } rlFramebufferAttachTextureType;

    // Keyboard keys (US keyboard layout)
// NOTE: Use GetKeyPressed() to allow redefining
// required keys for alternative layouts
    typedef enum {
        KEY_NULL = 0,        // Key: NULL, used for no key pressed
        // Alphanumeric keys
        KEY_APOSTROPHE = 39,       // Key: '
        KEY_COMMA = 44,       // Key: ,
        KEY_MINUS = 45,       // Key: -
        KEY_PERIOD = 46,       // Key: .
        KEY_SLASH = 47,       // Key: /
        KEY_ZERO = 48,       // Key: 0
        KEY_ONE = 49,       // Key: 1
        KEY_TWO = 50,       // Key: 2
        KEY_THREE = 51,       // Key: 3
        KEY_FOUR = 52,       // Key: 4
        KEY_FIVE = 53,       // Key: 5
        KEY_SIX = 54,       // Key: 6
        KEY_SEVEN = 55,       // Key: 7
        KEY_EIGHT = 56,       // Key: 8
        KEY_NINE = 57,       // Key: 9
        KEY_SEMICOLON = 59,       // Key: ;
        KEY_EQUAL = 61,       // Key: =
        KEY_A = 65,       // Key: A | a
        KEY_B = 66,       // Key: B | b
        KEY_C = 67,       // Key: C | c
        KEY_D = 68,       // Key: D | d
        KEY_E = 69,       // Key: E | e
        KEY_F = 70,       // Key: F | f
        KEY_G = 71,       // Key: G | g
        KEY_H = 72,       // Key: H | h
        KEY_I = 73,       // Key: I | i
        KEY_J = 74,       // Key: J | j
        KEY_K = 75,       // Key: K | k
        KEY_L = 76,       // Key: L | l
        KEY_M = 77,       // Key: M | m
        KEY_N = 78,       // Key: N | n
        KEY_O = 79,       // Key: O | o
        KEY_P = 80,       // Key: P | p
        KEY_Q = 81,       // Key: Q | q
        KEY_R = 82,       // Key: R | r
        KEY_S = 83,       // Key: S | s
        KEY_T = 84,       // Key: T | t
        KEY_U = 85,       // Key: U | u
        KEY_V = 86,       // Key: V | v
        KEY_W = 87,       // Key: W | w
        KEY_X = 88,       // Key: X | x
        KEY_Y = 89,       // Key: Y | y
        KEY_Z = 90,       // Key: Z | z
        KEY_LEFT_BRACKET = 91,       // Key: [
        KEY_BACKSLASH = 92,       // Key: '\'
        KEY_RIGHT_BRACKET = 93,       // Key: ]
        KEY_GRAVE = 96,       // Key: `
        // Function keys
        KEY_SPACE = 32,       // Key: Space
        KEY_ESCAPE = 256,      // Key: Esc
        KEY_ENTER = 257,      // Key: Enter
        KEY_TAB = 258,      // Key: Tab
        KEY_BACKSPACE = 259,      // Key: Backspace
        KEY_INSERT = 260,      // Key: Ins
        KEY_DELETE = 261,      // Key: Del
        KEY_RIGHT = 262,      // Key: Cursor right
        KEY_LEFT = 263,      // Key: Cursor left
        KEY_DOWN = 264,      // Key: Cursor down
        KEY_UP = 265,      // Key: Cursor up
        KEY_PAGE_UP = 266,      // Key: Page up
        KEY_PAGE_DOWN = 267,      // Key: Page down
        KEY_HOME = 268,      // Key: Home
        KEY_END = 269,      // Key: End
        KEY_CAPS_LOCK = 280,      // Key: Caps lock
        KEY_SCROLL_LOCK = 281,      // Key: Scroll down
        KEY_NUM_LOCK = 282,      // Key: Num lock
        KEY_PRINT_SCREEN = 283,      // Key: Print screen
        KEY_PAUSE = 284,      // Key: Pause
        KEY_F1 = 290,      // Key: F1
        KEY_F2 = 291,      // Key: F2
        KEY_F3 = 292,      // Key: F3
        KEY_F4 = 293,      // Key: F4
        KEY_F5 = 294,      // Key: F5
        KEY_F6 = 295,      // Key: F6
        KEY_F7 = 296,      // Key: F7
        KEY_F8 = 297,      // Key: F8
        KEY_F9 = 298,      // Key: F9
        KEY_F10 = 299,      // Key: F10
        KEY_F11 = 300,      // Key: F11
        KEY_F12 = 301,      // Key: F12
        KEY_LEFT_SHIFT = 340,      // Key: Shift left
        KEY_LEFT_CONTROL = 341,      // Key: Control left
        KEY_LEFT_ALT = 342,      // Key: Alt left
        KEY_LEFT_SUPER = 343,      // Key: Super left
        KEY_RIGHT_SHIFT = 344,      // Key: Shift right
        KEY_RIGHT_CONTROL = 345,      // Key: Control right
        KEY_RIGHT_ALT = 346,      // Key: Alt right
        KEY_RIGHT_SUPER = 347,      // Key: Super right
        KEY_KB_MENU = 348,      // Key: KB menu
        // Keypad keys
        KEY_KP_0 = 320,      // Key: Keypad 0
        KEY_KP_1 = 321,      // Key: Keypad 1
        KEY_KP_2 = 322,      // Key: Keypad 2
        KEY_KP_3 = 323,      // Key: Keypad 3
        KEY_KP_4 = 324,      // Key: Keypad 4
        KEY_KP_5 = 325,      // Key: Keypad 5
        KEY_KP_6 = 326,      // Key: Keypad 6
        KEY_KP_7 = 327,      // Key: Keypad 7
        KEY_KP_8 = 328,      // Key: Keypad 8
        KEY_KP_9 = 329,      // Key: Keypad 9
        KEY_KP_DECIMAL = 330,      // Key: Keypad .
        KEY_KP_DIVIDE = 331,      // Key: Keypad /
        KEY_KP_MULTIPLY = 332,      // Key: Keypad *
        KEY_KP_SUBTRACT = 333,      // Key: Keypad -
        KEY_KP_ADD = 334,      // Key: Keypad +
        KEY_KP_ENTER = 335,      // Key: Keypad Enter
        KEY_KP_EQUAL = 336,      // Key: Keypad =
        // Android key buttons
        KEY_BACK = 4,        // Key: Android back button
        KEY_MENU = 82,       // Key: Android menu button
        KEY_VOLUME_UP = 24,       // Key: Android volume up button
        KEY_VOLUME_DOWN = 25        // Key: Android volume down button
    } KeyboardKey;

// Mouse buttons
    typedef enum {
        MOUSE_BUTTON_LEFT = 0,       // Mouse button left
        MOUSE_BUTTON_RIGHT = 1,       // Mouse button right
        MOUSE_BUTTON_MIDDLE = 2,       // Mouse button middle (pressed wheel)
        MOUSE_BUTTON_SIDE = 3,       // Mouse button side (advanced mouse device)
        MOUSE_BUTTON_EXTRA = 4,       // Mouse button extra (advanced mouse device)
        MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
        MOUSE_BUTTON_BACK = 6,       // Mouse button back (advanced mouse device)
    } MouseButton;

    // Mouse cursor
    typedef enum {
        MOUSE_CURSOR_DEFAULT = 0,     // Default pointer shape
        MOUSE_CURSOR_ARROW = 1,     // Arrow shape
        MOUSE_CURSOR_IBEAM = 2,     // Text writing cursor shape
        MOUSE_CURSOR_CROSSHAIR = 3,     // Cross shape
        MOUSE_CURSOR_POINTING_HAND = 4,     // Pointing hand cursor
        MOUSE_CURSOR_RESIZE_EW = 5,     // Horizontal resize/move arrow shape
        MOUSE_CURSOR_RESIZE_NS = 6,     // Vertical resize/move arrow shape
        MOUSE_CURSOR_RESIZE_NWSE = 7,     // Top-left to bottom-right diagonal resize/move arrow shape
        MOUSE_CURSOR_RESIZE_NESW = 8,     // The top-right to bottom-left diagonal resize/move arrow shape
        MOUSE_CURSOR_RESIZE_ALL = 9,     // The omnidirectional resize/move cursor shape
        MOUSE_CURSOR_NOT_ALLOWED = 10     // The operation-not-allowed shape
    } MouseCursor;

    // Gamepad buttons
    typedef enum {
        GAMEPAD_BUTTON_UNKNOWN = 0,         // Unknown button, just for error checking
        GAMEPAD_BUTTON_LEFT_FACE_UP,        // Gamepad left DPAD up button
        GAMEPAD_BUTTON_LEFT_FACE_RIGHT,     // Gamepad left DPAD right button
        GAMEPAD_BUTTON_LEFT_FACE_DOWN,      // Gamepad left DPAD down button
        GAMEPAD_BUTTON_LEFT_FACE_LEFT,      // Gamepad left DPAD left button
        GAMEPAD_BUTTON_RIGHT_FACE_UP,       // Gamepad right button up (i.e. PS3: Triangle, Xbox: Y)
        GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,    // Gamepad right button right (i.e. PS3: Square, Xbox: X)
        GAMEPAD_BUTTON_RIGHT_FACE_DOWN,     // Gamepad right button down (i.e. PS3: Cross, Xbox: A)
        GAMEPAD_BUTTON_RIGHT_FACE_LEFT,     // Gamepad right button left (i.e. PS3: Circle, Xbox: B)
        GAMEPAD_BUTTON_LEFT_TRIGGER_1,      // Gamepad top/back trigger left (first), it could be a trailing button
        GAMEPAD_BUTTON_LEFT_TRIGGER_2,      // Gamepad top/back trigger left (second), it could be a trailing button
        GAMEPAD_BUTTON_RIGHT_TRIGGER_1,     // Gamepad top/back trigger right (one), it could be a trailing button
        GAMEPAD_BUTTON_RIGHT_TRIGGER_2,     // Gamepad top/back trigger right (second), it could be a trailing button
        GAMEPAD_BUTTON_MIDDLE_LEFT,         // Gamepad center buttons, left one (i.e. PS3: Select)
        GAMEPAD_BUTTON_MIDDLE,              // Gamepad center buttons, middle one (i.e. PS3: PS, Xbox: XBOX)
        GAMEPAD_BUTTON_MIDDLE_RIGHT,        // Gamepad center buttons, right one (i.e. PS3: Start)
        GAMEPAD_BUTTON_LEFT_THUMB,          // Gamepad joystick pressed button left
        GAMEPAD_BUTTON_RIGHT_THUMB          // Gamepad joystick pressed button right
    } GamepadButton;

    // Gamepad axis
    typedef enum {
        GAMEPAD_AXIS_LEFT_X = 0,     // Gamepad left stick X axis
        GAMEPAD_AXIS_LEFT_Y = 1,     // Gamepad left stick Y axis
        GAMEPAD_AXIS_RIGHT_X = 2,     // Gamepad right stick X axis
        GAMEPAD_AXIS_RIGHT_Y = 3,     // Gamepad right stick Y axis
        GAMEPAD_AXIS_LEFT_TRIGGER = 4,     // Gamepad back trigger left, pressure level: [1..-1]
        GAMEPAD_AXIS_RIGHT_TRIGGER = 5      // Gamepad back trigger right, pressure level: [1..-1]
    } GamepadAxis;

    // Shader location index
    typedef enum {
		SHADER_LOC_VERTEX_POSITION = 0, // Shader location: vertex attribute: position
		SHADER_LOC_VERTEX_TEXCOORD01,   // Shader location: vertex attribute: texcoord01
		SHADER_LOC_VERTEX_TEXCOORD02,   // Shader location: vertex attribute: texcoord02
		SHADER_LOC_VERTEX_NORMAL,       // Shader location: vertex attribute: normal
		SHADER_LOC_VERTEX_TANGENT,      // Shader location: vertex attribute: tangent
		SHADER_LOC_VERTEX_COLOR,        // Shader location: vertex attribute: color
		SHADER_LOC_MATRIX_MVP,          // Shader location: matrix uniform: model-view-projection
		SHADER_LOC_MATRIX_VIEW,         // Shader location: matrix uniform: view (camera transform)
		SHADER_LOC_MATRIX_PROJECTION,   // Shader location: matrix uniform: projection
		SHADER_LOC_MATRIX_MODEL,        // Shader location: matrix uniform: model (transform)
		SHADER_LOC_MATRIX_NORMAL,       // Shader location: matrix uniform: normal
		SHADER_LOC_VECTOR_VIEW,         // Shader location: vector uniform: view
		SHADER_LOC_COLOR_DIFFUSE,       // Shader location: vector uniform: diffuse color
		SHADER_LOC_COLOR_SPECULAR,      // Shader location: vector uniform: specular color
		SHADER_LOC_COLOR_AMBIENT,       // Shader location: vector uniform: ambient color
		SHADER_LOC_MAP_ALBEDO,          // Shader location: sampler2d texture: albedo (same as: SHADER_LOC_MAP_DIFFUSE)
		SHADER_LOC_MAP_METALNESS,       // Shader location: sampler2d texture: metalness (same as: SHADER_LOC_MAP_SPECULAR)
		SHADER_LOC_MAP_NORMAL,          // Shader location: sampler2d texture: normal
		SHADER_LOC_MAP_ROUGHNESS,       // Shader location: sampler2d texture: roughness
		SHADER_LOC_MAP_OCCLUSION,       // Shader location: sampler2d texture: occlusion
		SHADER_LOC_MAP_EMISSION,        // Shader location: sampler2d texture: emission
		SHADER_LOC_MAP_HEIGHT,          // Shader location: sampler2d texture: height
		SHADER_LOC_MAP_CUBEMAP,         // Shader location: samplerCube texture: cubemap
		SHADER_LOC_MAP_IRRADIANCE,      // Shader location: samplerCube texture: irradiance
		SHADER_LOC_MAP_PREFILTER,       // Shader location: samplerCube texture: prefilter
		SHADER_LOC_MAP_BRDF,            // Shader location: sampler2d texture: brdf
		SHADER_LOC_VERTEX_BONEIDS,      // Shader location: vertex attribute: boneIds
		SHADER_LOC_VERTEX_BONEWEIGHTS,  // Shader location: vertex attribute: boneWeights
		SHADER_LOC_BONE_MATRICES,       // Shader location: array of matrices uniform: boneMatrices
		SHADER_LOC_VERTEX_INSTANCE_TX   // Shader location: vertex attribute: instanceTransform
    } ShaderLocationIndex;

    // Shader uniform data type
    typedef enum {
        SHADER_UNIFORM_FLOAT = 0,       // Shader uniform type: float
        SHADER_UNIFORM_VEC2,            // Shader uniform type: vec2 (2 float)
        SHADER_UNIFORM_VEC3,            // Shader uniform type: vec3 (3 float)
        SHADER_UNIFORM_VEC4,            // Shader uniform type: vec4 (4 float)
        SHADER_UNIFORM_INT,             // Shader uniform type: int
        SHADER_UNIFORM_IVEC2,           // Shader uniform type: ivec2 (2 int)
        SHADER_UNIFORM_IVEC3,           // Shader uniform type: ivec3 (3 int)
        SHADER_UNIFORM_IVEC4,           // Shader uniform type: ivec4 (4 int)
        SHADER_UNIFORM_UINT,            // Shader uniform type: unsigned int
        SHADER_UNIFORM_UIVEC2,          // Shader uniform type: uivec2 (2 unsigned int)
        SHADER_UNIFORM_UIVEC3,          // Shader uniform type: uivec3 (3 unsigned int)
        SHADER_UNIFORM_UIVEC4,          // Shader uniform type: uivec4 (4 unsigned int)
        SHADER_UNIFORM_SAMPLER2D        // Shader uniform type: sampler2d
    } ShaderUniformDataType;

    // Shader attribute data types
    typedef enum {
        SHADER_ATTRIB_FLOAT = 0,        // Shader attribute type: float
        SHADER_ATTRIB_VEC2,             // Shader attribute type: vec2 (2 float)
        SHADER_ATTRIB_VEC3,             // Shader attribute type: vec3 (3 float)
        SHADER_ATTRIB_VEC4              // Shader attribute type: vec4 (4 float)
    } ShaderAttributeDataType;

    // Pixel formats
    // NOTE: Support depends on OpenGL version and platform
    typedef enum {
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1, // 8 bit per pixel (no alpha)
        PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,    // 8*2 bpp (2 channels)
        PIXELFORMAT_UNCOMPRESSED_R5G6B5,        // 16 bpp
        PIXELFORMAT_UNCOMPRESSED_R8G8B8,        // 24 bpp
        PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,      // 16 bpp (1 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,      // 16 bpp (4 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,      // 32 bpp
        PIXELFORMAT_UNCOMPRESSED_R32,           // 32 bpp (1 channel - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32,     // 32*3 bpp (3 channels - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,  // 32*4 bpp (4 channels - float)
        PIXELFORMAT_UNCOMPRESSED_R16,           // 16 bpp (1 channel - half float)
        PIXELFORMAT_UNCOMPRESSED_R16G16B16,     // 16*3 bpp (3 channels - half float)
        PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,  // 16*4 bpp (4 channels - half float)
        PIXELFORMAT_COMPRESSED_DXT1_RGB,        // 4 bpp (no alpha)
        PIXELFORMAT_COMPRESSED_DXT1_RGBA,       // 4 bpp (1 bit alpha)
        PIXELFORMAT_COMPRESSED_DXT3_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_DXT5_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_ETC1_RGB,        // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_RGB,        // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,   // 8 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGB,        // 4 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGBA,       // 4 bpp
        PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,   // 8 bpp
        PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA    // 2 bpp
    } PixelFormat;

    // Texture parameters: filter mode
    // NOTE 1: Filtering considers mipmaps if available in the texture
    // NOTE 2: Filter is accordingly set for minification and magnification
    typedef enum {
        TEXTURE_FILTER_POINT = 0,               // No filter, just pixel approximation
        TEXTURE_FILTER_BILINEAR,                // Linear filtering
        TEXTURE_FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
        TEXTURE_FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
        TEXTURE_FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
        TEXTURE_FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
    } TextureFilter;

    // Texture parameters: wrap mode
    typedef enum {
        TEXTURE_WRAP_REPEAT = 0,                // Repeats texture in tiled mode
        TEXTURE_WRAP_CLAMP,                     // Clamps texture to edge pixel in tiled mode
        TEXTURE_WRAP_MIRROR_REPEAT,             // Mirrors and repeats the texture in tiled mode
        TEXTURE_WRAP_MIRROR_CLAMP               // Mirrors and clamps to border the texture in tiled mode
    } TextureWrap;

    // Cubemap layouts
    typedef enum {
        CUBEMAP_LAYOUT_AUTO_DETECT = 0,         // Automatically detect layout type
        CUBEMAP_LAYOUT_LINE_VERTICAL,           // Layout is defined by a vertical line with faces
        CUBEMAP_LAYOUT_LINE_HORIZONTAL,         // Layout is defined by a horizontal line with faces
        CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR,     // Layout is defined by a 3x4 cross with cubemap faces
        CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE     // Layout is defined by a 4x3 cross with cubemap faces
    } CubemapLayout;

    // Material map index
    typedef enum {
        MATERIAL_MAP_ALBEDO = 0,        // Albedo material (same as: MATERIAL_MAP_DIFFUSE)
        MATERIAL_MAP_METALNESS,         // Metalness material (same as: MATERIAL_MAP_SPECULAR)
        MATERIAL_MAP_NORMAL,            // Normal material
        MATERIAL_MAP_ROUGHNESS,         // Roughness material
        MATERIAL_MAP_OCCLUSION,         // Ambient occlusion material
        MATERIAL_MAP_EMISSION,          // Emission material
        MATERIAL_MAP_HEIGHT,            // Heightmap material
        MATERIAL_MAP_CUBEMAP,           // Cubemap material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
        MATERIAL_MAP_IRRADIANCE,        // Irradiance material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
        MATERIAL_MAP_PREFILTER,         // Prefilter material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
        MATERIAL_MAP_BRDF               // Brdf material
    } MaterialMapIndex;

    // Color blending modes (pre-defined)
    typedef enum {
        RL_BLEND_ALPHA = 0,                 // Blend textures considering alpha (default)
        RL_BLEND_ADDITIVE,                  // Blend textures adding colors
        RL_BLEND_MULTIPLIED,                // Blend textures multiplying colors
        RL_BLEND_ADD_COLORS,                // Blend textures adding colors (alternative)
        RL_BLEND_SUBTRACT_COLORS,           // Blend textures subtracting colors (alternative)
        RL_BLEND_ALPHA_PREMULTIPLY,         // Blend premultiplied textures considering alpha
        RL_BLEND_CUSTOM,                    // Blend textures using custom src/dst factors (use rlSetBlendFactors())
        RL_BLEND_CUSTOM_SEPARATE            // Blend textures using custom src/dst factors (use rlSetBlendFactorsSeparate())
    } rlBlendMode;

    void InitWindow(int width, int height, const char* title);
    bool WindowShouldClose();
    void CloseWindow();
    bool IsWindowReady();
    bool IsWindowFullscreen();
    bool IsWindowHidden();
    bool IsWindowMinimized();
    bool IsWindowMaximized();
    bool IsWindowFocused();
    bool IsWindowResized();
    bool IsWindowState(unsigned int flag);
    void SetWindowState(unsigned int flags);
    void ClearWindowState(unsigned int flags);
    void ToggleFullscreen();
    void ToggleBorderlessWindowed();
    void MaximizeWindow();
    void MinimizeWindow();
    void RestoreWindow();
    void SetWindowIcon(Image image);
    void SetWindowIcons(Image* images, int count);
    void SetWindowTitle(const char* title);
    void SetWindowPosition(int x, int y);
    void SetWindowMonitor(int monitor);
    void SetWindowMinSize(int width, int height);
    void SetWindowSize(int width, int height);
    void SetWindowOpacity(float opacity);
    void* GetWindowHandle();
    int GetScreenWidth();
    int GetScreenHeight();
    int GetRenderWidth();
    int GetRenderHeight();
    int GetMonitorCount();
    int GetCurrentMonitor();
    Vector2 GetMonitorPosition(int monitor);
    int GetMonitorWidth(int monitor);
    int GetMonitorHeight(int monitor);
    int GetMonitorPhysicalWidth(int monitor);
    int GetMonitorPhysicalHeight(int monitor);
    int GetMonitorRefreshRate(int monitor);
    Vector2 GetWindowPosition();
    Vector2 GetWindowScaleDPI();
    const char* GetMonitorName(int monitor);
    void SetClipboardText(const char* text);
    const char* GetClipboardText();
    void EnableEventWaiting();
    void DisableEventWaiting();

    void SetConfigFlags(unsigned int flags);

    // Screen-space-related functions
    Ray GetScreenToWorldRay(Vector2 mousePosition, Camera camera);
    Matrix GetCameraMatrix(Camera camera);
    Matrix GetCameraMatrix2D(Camera2D camera);
    Vector2 GetWorldToScreen(Vector3 position, Camera camera);
    Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera);
    Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height);
    Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);

    // Timing-related functions
    void SetTargetFPS(int fps);
    int GetFPS();
    float GetFrameTime();
    double GetTime();

    // Cursor-related functions
    void ShowCursor(void);
    void HideCursor(void);
    bool IsCursorHidden(void);
    void EnableCursor(void);
    void DisableCursor(void);
    bool IsCursorOnScreen(void);

    // Drawing-related functions
    void ClearBackground(Color color);
    void BeginDrawing(void);
    void EndDrawing(void);
    void BeginMode2D(Camera2D camera);
    void EndMode2D(void);
    void BeginMode3D(Camera3D camera);
    void EndMode3D(void);
    void BeginTextureMode(RenderTexture2D target);
    void EndTextureMode(void);
    void BeginShaderMode(Shader shader);
    void EndShaderMode(void);
    void BeginBlendMode(int mode);
    void EndBlendMode(void);
    void BeginScissorMode(int x, int y, int width, int height);
    void EndScissorMode(void);
    void BeginVrStereoMode(VrStereoConfig config);
    void EndVrStereoMode(void);

    void DrawCube(Vector3 position, float width, float height, float length, Color color);
    void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
    void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color);

    void Draw3DBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint);
    void Draw3DBillboard(Camera camera, Texture2D texture, Vector3 position, float size, Color tint);

    void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color);
    void DrawFilledCircle3D(Vector3 center, float radius, Color color);
    void DrawFilledCircle3DAligned(Vector3 center, float radius, Vector3 normal, Color color);

    // Shaders
    Shader LoadShader(const char* vsFileName, const char* fsFileName);
    void UnloadShader(Shader shader);
    void SetShaderValue(Shader shader, int locIndex, const void* value, int uniformType);
    void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture);
    int GetShaderLocation(Shader shader, const char* uniformName);
    void SetShaderValueMatrix(Shader shader, int locIndex, Matrix matrix);

    // Image loading functions
    Image LoadImage(const char* fileName);
    Image LoadImageFromMemory(const char* fileType, const unsigned char* fileData, int dataSize);
    Image LoadImageFromTexture(Texture2D texture);
    void UnloadImage(Image image);

    bool ExportImage(Image image, const char* fileName);
    void ImageResize(Image* image, int newWidth, int newHeight);
    Color* LoadImageColors(Image image);
    void UnloadImageColors(Color* colors);
    void ImageFormat(Image* image, int newFormat);

    // Texture loading functions
    Texture2D LoadTexture(const char* fileName);
    Texture2D LoadTextureFromImage(Image image);
    TextureCubemap LoadTextureCubemap(Image image, int layout);
    RenderTexture2D LoadRenderTexture(int width, int height);
    bool IsTextureValid(Texture2D texture);
    void UnloadTexture(Texture2D texture);
    bool IsRenderTextureValid(RenderTexture2D target);
    void UnloadRenderTexture(RenderTexture2D target);
    void UpdateTexture(Texture2D texture, const void* pixels);
    void UpdateTextureRec(Texture2D texture, Rectangle rec, const void* pixels);

    // Texture Drawing
    void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
    void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
    // A modified version of DrawTexturePro to flip the quad
    void DrawTextureProFlipped(Texture2D _texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
    // A modified version of DrawRectanglePro to flip the quad
    void DrawRectangleProFlipped(Rectangle rec, Vector2 origin, float rotation, Color color);
    // A modified version of DrawRectangleLinesEx
    void DrawRectangleOutline(Rectangle rec, Vector2 origin, float rotation, float lineThick, Color color);
    void DrawCircleLinesV(Vector2 center, float radius, Color color);
    // A modified version of DrawCircleSector to flip the quad
    void DrawCircleSectorFlipped(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);

    // Texture Misc
    void SetTextureFilter(Texture2D texture, int filter);

    // Image Drawing
    void ImageDrawRectangle(Image* dst, int posX, int posY, int width, int height, Color color);

    // Input-related functions: keyboard
    bool IsKeyPressed(int key);
    bool IsKeyDown(int key);
    bool IsKeyReleased(int key);
    bool IsKeyUp(int key);
    void SetExitKey(int key);
    int GetKeyPressed(void);
    int GetCharPressed(void);

    // Input-related functions: gamepads
    bool IsGamepadAvailable(int gamepad);
    const char* GetGamepadName(int gamepad);
    bool IsGamepadButtonPressed(int gamepad, int button);
    bool IsGamepadButtonDown(int gamepad, int button);
    bool IsGamepadButtonReleased(int gamepad, int button);
    bool IsGamepadButtonUp(int gamepad, int button);
    int GetGamepadButtonPressed(void);
    int GetGamepadAxisCount(int gamepad);
    float GetGamepadAxisMovement(int gamepad, int axis);
    int SetGamepadMappings(const char* mappings);

    // Input-related functions: mouse
    bool IsMouseButtonPressed(int button);
    bool IsMouseButtonDown(int button);
    bool IsMouseButtonReleased(int button);
    bool IsMouseButtonUp(int button);
    int GetMouseX(void);
    int GetMouseY(void);
    Vector2 GetMousePosition(void);
    Vector2 GetMouseDelta(void);
    void SetMousePosition(int x, int y);
    void SetMouseOffset(int offsetX, int offsetY);
    void SetMouseScale(float scaleX, float scaleY);
    float GetMouseWheelMove(void);
    Vector2 GetMouseWheelMoveV(void);
    void SetMouseCursor(int cursor);

    // Input-related functions: touch
    int GetTouchX(void);
    int GetTouchY(void);
    Vector2 GetTouchPosition(int index);
    int GetTouchPointId(int index);
    int GetTouchPointCount(void);


    // Camera management functions
    void UpdateCamera(Camera* camera, int mode);
    //void UpdateCameraPro(Camera* camera, Vector3 movement, Vector3 rotation, float zoom);


    // Vector Math
    Vector2 Vector2Add(Vector2 v1, Vector2 v2);
    Vector2 Vector2Subtract(Vector2 v1, Vector2 v2);
    Vector3 Vector3Add(Vector3 v1, Vector3 v2);
    Vector3 Vector3Subtract(Vector3 v1, Vector3 v2);
    Vector3 Vector3Scale(Vector3 v, float scalar);
    float Vector3Distance(Vector3 v1, Vector3 v2);

    // NOTE: Rotation order is ZYX
    Quaternion QuaternionFromEuler(float pitch, float yaw, float roll);

    // Get the Euler angles equivalent to quaternion (roll, pitch, yaw)
    // NOTE: Angles are returned in a Vector3 struct in radians
    Vector3 QuaternionToEuler(Quaternion q);

    // Filesystem-related functions
    bool IsFileDropped(void);
    FilePathList LoadDroppedFiles(void);
    void UnloadDroppedFiles(FilePathList files);


    void DrawGrid(int slices, float spacing);


    // Basic 2D Collisions
    bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
    bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);
    bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);

    // Basic 3D Collisions
    RayCollision GetRayCollisionBox(Ray ray, BoundingBox box);

    // Model animations loading/unloading functions
    //ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount);            // Load model animations from file
    //void UpdateModelAnimation(Model model, ModelAnimation anim, int frame);               // Update model animation pose --- Moved to RaylibModelWrapper
    void UnloadModelAnimation(ModelAnimation anim);                                       // Unload animation data
    void UnloadModelAnimations(ModelAnimation* animations, int animCount);                // Unload animation array data
    //bool IsModelAnimationValid(Model model, ModelAnimation anim);                         // Check model animation skeleton match --- Moved to RaylibModelWrapper

    const char* GetWorkingDirectory(void);                      // Get current working directory (uses static string)

    // Materials
    //Material LoadMaterial();
    void UnloadMaterial(Material material);


    // Audio
    
    // Wave/Sound loading/unloading functions
    void InitAudioDevice(void);                                     // Initialize audio device and context
    void CloseAudioDevice(void);                                    // Close the audio device and context
    bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
    void SetMasterVolume(float volume);                             // Set master volume (listener)
    float GetMasterVolume(void);                                    // Get master volume (listener)

    Wave LoadWave(const char* fileName);                            // Load wave data from file
    bool IsWaveReady(Wave wave);                                    // Checks if wave data is ready
    Sound LoadSound(const char* fileName);                          // Load sound from file
    Sound LoadSoundFromWave(Wave wave);                             // Load sound from wave data
    bool IsSoundReady(Sound sound);                                 // Checks if a sound is ready
    void UpdateSound(Sound sound, const void* data, int sampleCount); // Update sound buffer with new data
    void UnloadWave(Wave wave);                                     // Unload wave data
    void UnloadSound(Sound sound);                                  // Unload sound

    // Wave/Sound management functions
    void PlaySound(Sound sound);                                    // Play a sound
    void StopSound(Sound sound);                                    // Stop playing a sound
    void PauseSound(Sound sound);                                   // Pause a sound
    void ResumeSound(Sound sound);                                  // Resume a paused sound
    bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
    void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
    void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)
    void SetSoundPan(Sound sound, float pan);                       // Set pan for a sound (0.5 is center)

    // Music management functions
    Music LoadMusicStream(const char* fileName);                    // Load music stream from file
    Music LoadMusicStreamFromMemory(const char* fileType, const unsigned char* data, int dataSize); // Load music stream from data
    bool IsMusicReady(Music music);                                 // Checks if a music stream is ready
    void UnloadMusicStream(Music music);                            // Unload music stream
    void PlayMusicStream(Music music);                              // Start music playing
    bool IsMusicStreamPlaying(Music music);                         // Check if music is playing
    void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
    void StopMusicStream(Music music);                              // Stop music playing
    void PauseMusicStream(Music music);                             // Pause music playing
    void ResumeMusicStream(Music music);                            // Resume playing paused music
    void SetMusicVolume(Music music, float volume);                 // Set volume for music (1.0 is max level)
    void SetMusicPitch(Music music, float pitch);                   // Set pitch for a music (1.0 is base level)
    void SetMusicPan(Music music, float pan);                       // Set pan for a music (0.5 is center)
    float GetMusicTimeLength(Music music);                          // Get music time length (in seconds)
    float GetMusicTimePlayed(Music music);                          // Get current music time played (in seconds)

    // Color functions
    Vector4 ColorNormalize(Color color); // Todo: Move to Color struct

    // rlgl functions
    void rlSetMatrixProjection(Matrix matrix);
    Matrix rlGetMatrixModelview();
    Matrix rlGetMatrixProjection();
    // NOTE: Fovy angle must be provided in radians
    Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane);
    Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up);
    void rlSetUniform(int locIndex, const void* value, int uniformType, int count);
    unsigned int rlLoadFramebuffer();
    void rlUnloadFramebuffer(unsigned int id);
    void rlEnableFramebuffer(unsigned int id);
    void rlDisableFramebuffer();
    void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel);
    bool rlFramebufferComplete(unsigned int id);
    unsigned int rlLoadTexture(const void* data, int width, int height, int format, int mipmapCount);
    unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);
    void rlEnableTexture(unsigned int id);
    void rlDisableTexture();
    void rlEnableShader(unsigned int id);
    void rlDisableShader();
    void rlActiveTextureSlot(int slot);
    void rlSetUniformSampler(int locIndex, unsigned int textureId);
    void rlSetUniformMatrix(int locIndex, Matrix matrix);
    void rlClearScreenBuffers();
    void rlDisableColorBlend();
    void rlDisableColorBlend();
    void rlBegin(int mode);
    void rlEnd();
    void rlVertex3f(float x, float y, float z);
    void rlDrawRenderBatchActive();
    void rlSetBlendMode(int mode);

    // raymath functions
    Matrix MatrixOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane);
    Matrix MatrixMultiply(Matrix left, Matrix right);
    Vector3 Vector3Normalize(Vector3 v);
    Matrix MatrixRotateXYZ(Vector3 angle);
    Vector3 Vector3Transform(Vector3 v, Matrix matrix);
    Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2);
    Matrix MatrixInvert(Matrix matrix);

    // Model functions
    Model LoadModelFromMesh(Mesh mesh);
    void UploadMesh(Mesh* mesh, bool dynamic);
    void UnloadMesh(Mesh mesh);


    // Image functions
    Image GenImageColor(int width, int height, Color color);
    void UnloadImage(Image image);

    // ImGui Raylib
    bool ImGui_ImplRaylib_Init();
    void ImGui_ImplRaylib_Shutdown();
    void ImGui_ImplRaylib_NewFrame();
    void ImGui_ImplRaylib_RenderDrawData(ImDrawData* draw_data);
    bool ImGui_ImplRaylib_ProcessEvents();

    void rlImGuiSetup(bool darkTheme);
    void rlImGuiBegin();
    void rlImGuiEnd();
    void rlImGuiShutdown();
    void rlImGuiBeginInitImGui();
    void rlImGuiEndInitImGui();
    void rlImGuiReloadFonts();
    void rlImGuiBeginDelta(float deltaTime);
    void rlImGuiImage(const Texture* image);
    void rlImGuiImageSize(const Texture* image, int width, int height);
    void rlImGuiImageSizeV(const Texture* image, Vector2 size);
    void rlImGuiImageSizeTintV(const Texture* image, Vector2 size, Vector4 tint);
    void rlImGuiImageRect(const Texture* image, int destWidth, int destHeight, Rectangle sourceRect);
    void rlImGuiImageRenderTexture(const RenderTexture* image);
    void rlImGuiImageRenderTextureFit(const RenderTexture* image, bool center);
    bool rlImGuiImageButton(const char* name, const Texture* image);
    bool rlImGuiImageButtonSize(const char* name, const Texture* image, ImVec2 size);
    bool rlImGuiImageButtonSizeTint(const char* name, const Texture* image, ImVec2 size, ImVec4 tint);
}