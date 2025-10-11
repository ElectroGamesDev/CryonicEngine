#include "RaylibWrapper.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cstring>
#include "imgui_impl_raylib.h"
#include "rlImGui.h"

#ifndef SMOOTH_CIRCLE_ERROR_RATE
#define SMOOTH_CIRCLE_ERROR_RATE    0.5f      // Circle error rate
#endif

// Todo: create functions to convert bigger structs like model, mesh, bone, etc.

namespace RaylibWrapper {
    void InitWindow(int width, int height, const char* title) {
        ::InitWindow(width, height, title);
    }

    bool WindowShouldClose() {
        return ::WindowShouldClose();
    }

    void CloseWindow() {
        ::CloseWindow();
    }

    bool IsWindowReady() {
        return ::IsWindowReady();
    }

    bool IsWindowFullscreen() {
        return ::IsWindowFullscreen();
    }

    bool IsWindowHidden() {
        return ::IsWindowHidden();
    }

    bool IsWindowMinimized() {
        return ::IsWindowMinimized();
    }

    bool IsWindowMaximized() {
        return ::IsWindowMaximized();
    }

    bool IsWindowFocused() {
        return ::IsWindowFocused();
    }

    bool IsWindowResized() {
        return ::IsWindowResized();
    }

    bool IsWindowState(unsigned int flag) {
        return ::IsWindowState(flag);
    }

    void SetWindowState(unsigned int flags) {
        ::SetWindowState(flags);
    }

    void ClearWindowState(unsigned int flags) {
        ::ClearWindowState(flags);
    }

    void ToggleFullscreen() {
        ::ToggleFullscreen();
    }

    void ToggleBorderlessWindowed() {
        ::ToggleBorderlessWindowed();
    }

    void MaximizeWindow() {
        ::MaximizeWindow();
    }

    void MinimizeWindow() {
        ::MinimizeWindow();
    }

    void RestoreWindow() {
        ::RestoreWindow();
    }

    void SetWindowIcon(Image image) {
        ::SetWindowIcon({image.data, image.width, image.height, image.mipmaps, image.format});
    }

    //void SetWindowIcons(Image* images, int count) {
    //    ::SetWindowIcons(images, count);
    //}

    void SetWindowTitle(const char* title) {
        ::SetWindowTitle(title);
    }

    void SetWindowPosition(int x, int y) {
        ::SetWindowPosition(x, y);
    }

    void SetWindowMonitor(int monitor) {
        ::SetWindowMonitor(monitor);
    }

    void SetWindowMinSize(int width, int height) {
        ::SetWindowMinSize(width, height);
    }

    void SetWindowSize(int width, int height) {
        ::SetWindowSize(width, height);
    }

    void SetWindowOpacity(float opacity) {
        ::SetWindowOpacity(opacity);
    }

    void* GetWindowHandle() {
        return ::GetWindowHandle();
    }

    int GetScreenWidth() {
        return ::GetScreenWidth();
    }

    int GetScreenHeight() {
        return ::GetScreenHeight();
    }

    int GetRenderWidth() {
        return ::GetRenderWidth();
    }

    int GetRenderHeight() {
        return ::GetRenderHeight();
    }

    int GetMonitorCount() {
        return ::GetMonitorCount();
    }

    int GetCurrentMonitor() {
        return ::GetCurrentMonitor();
    }

    Vector2 GetMonitorPosition(int monitor) {
        ::Vector2 pos = ::GetMonitorPosition(monitor);
        return {pos.x, pos.y};
    }

    int GetMonitorWidth(int monitor) {
        return ::GetMonitorWidth(monitor);
    }

    int GetMonitorHeight(int monitor) {
        return ::GetMonitorHeight(monitor);
    }

    int GetMonitorPhysicalWidth(int monitor) {
        return ::GetMonitorPhysicalWidth(monitor);
    }

    int GetMonitorPhysicalHeight(int monitor) {
        return ::GetMonitorPhysicalHeight(monitor);
    }

    int GetMonitorRefreshRate(int monitor) {
        return ::GetMonitorRefreshRate(monitor);
    }

    Vector2 GetWindowPosition() {
        ::Vector2 pos = ::GetWindowPosition();
        return { pos.x, pos.y };
    }

    Vector2 GetWindowScaleDPI() {
        ::Vector2 pos = ::GetWindowScaleDPI();
        return {pos.x, pos.y};
    }

    const char* GetMonitorName(int monitor) {
        return ::GetMonitorName(monitor);
    }

    void SetClipboardText(const char* text) {
        ::SetClipboardText(text);
    }

    const char* GetClipboardText() {
        return ::GetClipboardText();
    }

    void EnableEventWaiting() {
        ::EnableEventWaiting();
    }

    void DisableEventWaiting() {
        ::DisableEventWaiting();
    }


    void SetConfigFlags(unsigned int flags) {
        ::SetConfigFlags(flags);
    }

    // Screen-space-related functions
    Ray GetScreenToWorldRay(Vector2 mousePosition, Camera camera) {
        ::Ray ray = ::GetScreenToWorldRay({ mousePosition.x, mousePosition.y }, { {camera.position.x, camera.position.y, camera.position.z}, {camera.target.x, camera.target.y, camera.target.z}, {camera.up.x, camera.up.y, camera.up.z}, camera.fovy, camera.projection });
        return { {ray.position.x, ray.position.y, ray.position.z}, {ray.direction.x, ray.direction.y, ray.direction.z} };
    }

    Matrix GetCameraMatrix(Camera camera) {
        ::Matrix matrix = ::GetCameraMatrix({ {camera.position.x, camera.position.y, camera.position.z}, {camera.target.x, camera.target.y, camera.target.z}, {camera.up.x, camera.up.y, camera.up.z}, camera.fovy, camera.projection });
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    Matrix GetCameraMatrix2D(Camera2D camera) {
        ::Matrix matrix = ::GetCameraMatrix2D({ {camera.offset.x, camera.offset.y}, {camera.target.x, camera.target.y}, camera.rotation, camera.zoom });
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    Vector2 GetWorldToScreen(Vector3 position, Camera camera) {
        ::Vector2 vector = ::GetWorldToScreen({position.x, position.y, position.z}, { {camera.position.x, camera.position.y, camera.position.z}, {camera.target.x, camera.target.y, camera.target.z}, {camera.up.x, camera.up.y, camera.up.z}, camera.fovy, camera.projection });
        return {vector.x, vector.y};
    }

    Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera) {
        ::Vector2 vector = ::GetScreenToWorld2D({ position.x, position.y }, { {camera.offset.x, camera.offset.y}, {camera.target.x, camera.target.y}, camera.rotation, camera.zoom });
        return { vector.x, vector.y };
    }

    Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height) {
        ::Vector2 vector = ::GetWorldToScreenEx({ position.x, position.y, position.z }, { {camera.position.x, camera.position.y, camera.position.z}, {camera.target.x, camera.target.y, camera.target.z}, {camera.up.x, camera.up.y, camera.up.z}, camera.fovy, camera.projection }, width, height);
        return { vector.x, vector.y };
    }

    Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera) {
        ::Vector2 vector = ::GetWorldToScreen2D({ position.x, position.y }, { {camera.offset.x, camera.offset.y}, {camera.target.x, camera.target.y}, camera.rotation, camera.zoom });
        return { vector.x, vector.y };
    }

    // Timing-related functions
    void SetTargetFPS(int fps) {
        ::SetTargetFPS(fps);
    }

    int GetFPS() {
        return ::GetFPS();
    }

    float GetFrameTime() {
        return ::GetFrameTime();
    }

    double GetTime() {
        return ::GetTime();
    }

    // Cursor-related functions
    void ShowCursor(void) {
        ::ShowCursor();
    }

    void HideCursor(void) {
        ::HideCursor();
    }

    bool IsCursorHidden(void) {
        return ::IsCursorHidden();
    }

    void EnableCursor(void) {
        ::EnableCursor();
    }

    void DisableCursor(void) {
        ::DisableCursor();
    }

    bool IsCursorOnScreen(void) {
        return ::IsCursorOnScreen();
    }

    // Drawing-related functions
    void ClearBackground(Color color) {
        ::ClearBackground({color.r, color.g, color.b, color.a});
    }

    void BeginDrawing(void) {
        ::BeginDrawing();
    }

    void EndDrawing(void) {
        ::EndDrawing();
    }

    void BeginMode2D(Camera2D camera) {
        ::BeginMode2D({ {camera.offset.x, camera.offset.y}, {camera.target.x, camera.target.y}, camera.rotation, camera.zoom });
    }

    void EndMode2D(void) {
        ::EndMode2D();
    }

    void BeginMode3D(Camera3D camera) {
        ::BeginMode3D({ {camera.position.x, camera.position.y, camera.position.z}, {camera.target.x, camera.target.y, camera.target.z}, {camera.up.x, camera.up.y, camera.up.z}, camera.fovy, camera.projection });
    }

    void EndMode3D(void) {
        ::EndMode3D();
    }

    void BeginTextureMode(RenderTexture2D target) {
        ::BeginTextureMode({ target.id, { target.texture.id, target.texture.width, target.texture.height, target.texture.mipmaps, target.texture.format }, { target.depth.id, target.depth.width, target.depth.height, target.depth.mipmaps, target.depth.format } });
    }

    void EndTextureMode(void) {
        ::EndTextureMode();
    }

    void BeginShaderMode(Shader shader) {
        ::BeginShaderMode({shader.id, shader.locs});
    }

    void EndShaderMode(void) {
        ::EndShaderMode();
    }

    void BeginBlendMode(int mode) {
        ::BeginBlendMode(mode);
    }

    void EndBlendMode(void) {
        ::EndBlendMode();
    }

    void BeginScissorMode(int x, int y, int width, int height) {
        ::BeginScissorMode(x, y, width, height);
    }

    void EndScissorMode(void) {
        ::EndScissorMode();
    }

    //void BeginVrStereoMode(VrStereoConfig config) {
    //    ::BeginVrStereoMode(config);
    //}

    void EndVrStereoMode(void) {
        ::EndVrStereoMode();
    }

    void DrawCube(Vector3 position, float width, float height, float length, Color color) {
        ::DrawCube({ position.x, position.y, position.z }, width, height, length, { color.r, color.g, color.b, color.a });
    }

    void DrawCubeWires(Vector3 position, float width, float height, float length, Color color) {
        ::DrawCubeWires({ position.x, position.y, position.z }, width, height, length, { color.r, color.g, color.b, color.a });
    }

    void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color) {
        ::DrawCylinderEx({ startPos.x, startPos.y, startPos.z }, { endPos.x, endPos.y, endPos.z }, startRadius, endRadius, sides, {color.r, color.g, color.b, color.a});
    }

    void Draw3DBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint)
    {
        ::rlPushMatrix();

        // get the camera view matrix
        ::Matrix mat = ::MatrixInvert(MatrixLookAt({ camera.position.x, camera.position.y, camera.position.z }, { camera.target.x, camera.target.y, camera.target.z }, { camera.up.x, camera.up.y, camera.up.z }));
        // peel off just the rotation
        ::Quaternion quat = ::QuaternionFromMatrix(mat);
        mat = ::QuaternionToMatrix(quat);

        // apply just the rotation
        rlMultMatrixf(MatrixToFloat(mat));

        ::Vector3 pos = {position.x, position.y, position.z};
        // translate backwards in the inverse rotated matrix to put the item where it goes in world space
        pos = ::Vector3Transform(pos, MatrixInvert(mat));
        ::rlTranslatef(pos.x, pos.y, pos.z);

        // draw the billboard
        float width = size.x / 2;
        float height = size.y / 2;

        Color color = WHITE;

        ::rlCheckRenderBatchLimit(6);

        ::rlSetTexture(texture.id);

        // draw quad
        ::rlBegin(RL_QUADS);
        ::rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        // Front Face
        ::rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer

        ::rlTexCoord2f((float)source.x / texture.width, (float)(source.y + source.height) / texture.height);
        ::rlVertex3f(-width, -height, 0);  // Bottom Left Of The Texture and Quad

        ::rlTexCoord2f((float)(source.x + source.width) / texture.width, (float)(source.y + source.height) / texture.height);
        ::rlVertex3f(+width, -height, 0);  // Bottom Right Of The Texture and Quad

        ::rlTexCoord2f((float)(source.x + source.width) / texture.width, (float)source.y / texture.height);
        ::rlVertex3f(+width, +height, 0);  // Top Right Of The Texture and Quad

        ::rlTexCoord2f((float)source.x / texture.width, (float)source.y / texture.height);
        ::rlVertex3f(-width, +height, 0);  // Top Left Of The Texture and Quad

        ::rlEnd();
        ::rlSetTexture(0);
        ::rlPopMatrix();
    }

    void Draw3DBillboard(Camera camera, Texture2D texture, Vector3 position, float size, Color tint) {
        Draw3DBillboardRec(camera, texture, { 0, 0, (float)texture.width, (float)texture.height }, position, { size, size }, tint);
    }

    void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color) {
		::DrawCircle3D({ center.x, center.y, center.z }, radius, { rotationAxis.x, rotationAxis.y, rotationAxis.z }, rotationAngle, { color.r, color.g, color.b, color.a });
    }

	void DrawFilledCircle3D(Vector3 center, float radius, Color color)
	{
		const int segments = 64;

		rlPushMatrix();
		rlTranslatef(center.x, center.y, center.z);
		rlDisableBackfaceCulling();

		rlBegin(RL_TRIANGLES);
		rlColor4ub(color.r, color.g, color.b, color.a);

		for (int i = 0; i < segments; i++)
		{
			float theta1 = ((float)i / segments) * 2.0f * PI;
			float theta2 = ((float)(i + 1) / segments) * 2.0f * PI;

			rlVertex3f(0.0f, 0.0f, 0.0f); // center
			rlVertex3f(cosf(theta1) * radius, 0.0f, sinf(theta1) * radius);
			rlVertex3f(cosf(theta2) * radius, 0.0f, sinf(theta2) * radius);
		}

		rlEnd();
		rlPopMatrix();
	}

    // Todo: This may not work properly
	void DrawFilledCircle3DAligned(Vector3 center, float radius, Vector3 normal, Color color)
	{
		const int segments = 64;
		::Vector3 rlNormal = { normal.x, normal.y, normal.z };

		// Normalize and ensure it's facing upward
        rlNormal = ::Vector3Normalize(rlNormal);
		if (rlNormal.y < 0) rlNormal = ::Vector3Negate(rlNormal);

		::Vector3 up = { 0, 1, 0 };
		::Vector3 axis = ::Vector3CrossProduct(up, rlNormal);
		float dot = Vector3DotProduct(up, rlNormal);
		float angle = acosf(fmaxf(fminf(dot, 1.0f), -1.0f)) * (180.0f / PI);

		if (Vector3Length(axis) < 0.0001f)
			axis = { 1, 0, 0 };

		// Lift slightly to prevent z-fighting
		center.y += 0.05f;

		rlPushMatrix();
		rlTranslatef(center.x, center.y, center.z);
		rlRotatef(angle, axis.x, axis.y, axis.z);

		rlBegin(RL_TRIANGLES);
		rlColor4ub(color.r, color.g, color.b, color.a);

		for (int i = 0; i < segments; i++)
		{
			float theta1 = (float)i / segments * 2.0f * PI;
			float theta2 = (float)(i + 1) / segments * 2.0f * PI;

			rlVertex3f(0.0f, 0.0f, 0.0f);
			rlVertex3f(cosf(theta1) * radius, 0.0f, sinf(theta1) * radius);
			rlVertex3f(cosf(theta2) * radius, 0.0f, sinf(theta2) * radius);
		}

		rlEnd();
		rlPopMatrix();
	}

    // Shaders
    Shader LoadShader(const char* vsFileName, const char* fsFileName) {
        ::Shader shader = ::LoadShader(vsFileName, fsFileName);
        return { shader.id, shader.locs };
    }

    void UnloadShader(Shader shader) {
        ::UnloadShader({ shader.id, shader.locs });
    }

    void SetShaderValue(Shader shader, int locIndex, const void* value, int uniformType) {
        ::SetShaderValue({ shader.id, shader.locs }, locIndex, value, uniformType);
    }

    void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture) {
        return ::SetShaderValueTexture({ shader.id, shader.locs }, locIndex, { texture.id, texture.width, texture.height, texture.mipmaps, texture.format });
    }

    int GetShaderLocation(Shader shader, const char* uniformName) {
        return ::GetShaderLocation({ shader.id, shader.locs }, uniformName);
    }

    void SetShaderValueMatrix(Shader shader, int locIndex, Matrix matrix) {
        ::SetShaderValueMatrix({ shader.id, shader.locs }, locIndex, { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 });
    }


    // Image functions
    Image GenImageColor(int width, int height, Color color) {
        ::Image image = ::GenImageColor(width, height, { color.r, color.g, color.b, color.a });
        return { image.data, image.width, image.height, image.mipmaps, image.format };
    }

    Image LoadImage(const char* fileName) {
        ::Image image = ::LoadImage(fileName);
		return { image.data, image.width, image.height, image.mipmaps, image.format };
    }

    Image LoadImageFromMemory(const char* fileType, const unsigned char* fileData, int dataSize)
    {
        ::Image image = ::LoadImageFromMemory(fileType, fileData, dataSize);
        return { image.data, image.width, image.height, image.mipmaps, image.format };
    }

    Image LoadImageFromTexture(Texture2D texture) {
        ::Image image = ::LoadImageFromTexture({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format });
        return { image.data, image.width, image.height, image.mipmaps, image.format };
    }

    void UnloadImage(Image image)
    {
        ::UnloadImage({ image.data, image.width, image.height, image.mipmaps, image.format });
    }

    bool ExportImage(Image image, const char* fileName) {
        return ::ExportImage({image.data, image.width, image.height, image.mipmaps, image.format}, fileName);
    }

    void ImageResize(Image* image, int newWidth, int newHeight) {
        ::Image img = { image->data, image->width, image->height, image->mipmaps, image->format };
        ::ImageResize(&img, newWidth, newHeight);
        image->data = img.data;
        image->width = img.width;
        image->height = img.height;
        image->mipmaps = img.mipmaps;
		image->format = img.format;
    }

    Color* LoadImageColors(Image image) {
        ::Color* colors = ::LoadImageColors({ image.data, image.width, image.height, image.mipmaps, image.format });
        Color* result = new Color[image.width * image.height];
        for (int i = 0; i < image.width * image.height; i++) {
            result[i] = { colors[i].r, colors[i].g, colors[i].b, colors[i].a };
        }
        ::RL_FREE(colors);
		return result;
    }

    void UnloadImageColors(Color* colors) {
        ::Color* cols = new ::Color[1]; // Dummy allocation to avoid passing nullptr
        cols[0] = { colors->r, colors->g, colors->b, colors->a };
        ::UnloadImageColors(cols);
        delete[] cols;
	}

    void ImageFormat(Image* image, int newFormat) {
        ::Image img = { image->data, image->width, image->height, image->mipmaps, image->format };
        ::ImageFormat(&img, newFormat);
        image->data = img.data;
        image->width = img.width;
        image->height = img.height;
        image->mipmaps = img.mipmaps;
		image->format = img.format;
    }


    // Texture loading functions
    Texture2D LoadTexture(const char* fileName) {
        ::Texture2D texture2D = ::LoadTexture(fileName);
        return { texture2D .id, texture2D.width, texture2D.height, texture2D.mipmaps, texture2D.format};
    }

    Texture2D LoadTextureFromImage(Image image) {
        ::Texture2D texture2D = ::LoadTextureFromImage({image.data, image.width, image.height, image.mipmaps, image.format});
        return { texture2D.id, texture2D.width, texture2D.height, texture2D.mipmaps, texture2D.format };
    }

    TextureCubemap LoadTextureCubemap(Image image, int layout) {
        ::TextureCubemap textureCubemap = ::LoadTextureCubemap({ image.data, image.width, image.height, image.mipmaps, image.format }, layout);
        return { textureCubemap.id, textureCubemap.width, textureCubemap.height, textureCubemap.mipmaps, textureCubemap.format };
    }

    RenderTexture2D LoadRenderTexture(int width, int height) {
        ::RenderTexture2D renderTexture2D = ::LoadRenderTexture(width, height);
        return { renderTexture2D.id, { renderTexture2D.texture.id, renderTexture2D.texture.width, renderTexture2D.texture.height, renderTexture2D.texture.mipmaps, renderTexture2D.texture.format }, { renderTexture2D.depth.id, renderTexture2D.depth.width, renderTexture2D.depth.height, renderTexture2D.depth.mipmaps, renderTexture2D.depth.format } };
    }

    bool IsTextureValid(Texture2D texture) {
        return ::IsTextureValid({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format });
    }

    void UnloadTexture(Texture2D texture) {
        ::UnloadTexture({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format });
    }

    bool IsRenderTextureValid(RenderTexture2D target) {
        return ::IsRenderTextureValid({ target.id, { target.texture.id, target.texture.width, target.texture.height, target.texture.mipmaps, target.texture.format }, { target.depth.id, target.depth.width, target.depth.height, target.depth.mipmaps, target.depth.format } });
    }

    void UnloadRenderTexture(RenderTexture2D target) {
        ::UnloadRenderTexture({ target.id, { target.texture.id, target.texture.width, target.texture.height, target.texture.mipmaps, target.texture.format }, { target.depth.id, target.depth.width, target.depth.height, target.depth.mipmaps, target.depth.format } });
    }

    void UpdateTexture(Texture2D texture, const void* pixels) {
        ::UpdateTexture({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format }, pixels);
    }

    void UpdateTextureRec(Texture2D texture, Rectangle rec, const void* pixels) {
        ::UpdateTextureRec({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format }, { rec.x, rec.y, rec.width, rec.height}, pixels);
    }

    // Texture Drawing
    void DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
        return ::DrawTexture({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format }, posX, posY, {tint.r, tint.g, tint.b, tint.a});
    }

    void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) {
        return ::DrawTexturePro({ texture.id, texture.width, texture.height, texture.mipmaps, texture.format }, { source.x, source.y, source.width, source.height }, { dest.x, dest.y, dest.width, dest.height }, {origin.x, origin.y}, rotation, { tint.r, tint.g, tint.b, tint.a });
    }

    // A modified version of DrawTexturePro to flip the quad
    void DrawTextureProFlipped(Texture2D _texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) {
        ::Texture2D texture = { _texture.id, _texture.width, _texture.height, _texture.mipmaps, _texture.format };
        //::Rectangle source = { _source.x, _source.y, _source.width, _source.height };
        //::Rectangle dest = { dest.x, dest.y, dest.width, dest.height };
        //::Vector2 origin = { origin.x, origin.y };
        //::Color tint = { tint.r, tint.g, tint.b, tint.a };
        // Check if texture is valid
        if (texture.id > 0)
        {
            float width = (float)texture.width;
            float height = (float)texture.height;

            bool flipX = false;

            if (source.width < 0) { flipX = true; source.width *= -1; }
            if (source.height < 0) source.y -= source.height;

            ::Vector2 topLeft = { 0 };
            ::Vector2 topRight = { 0 };
            ::Vector2 bottomLeft = { 0 };
            ::Vector2 bottomRight = { 0 };

            // Only calculate rotation if needed
            if (rotation == 0.0f)
            {
                float x = dest.x - origin.x;
                float y = dest.y - origin.y;
                topLeft = { x, y };
                topRight = { x + dest.width, y };
                bottomLeft = { x, y + dest.height };
                bottomRight = { x + dest.width, y + dest.height };
            }
            else
            {
                float sinRotation = sinf(rotation * DEG2RAD);
                float cosRotation = cosf(rotation * DEG2RAD);
                float x = dest.x;
                float y = dest.y;
                float dx = -origin.x;
                float dy = -origin.y;

                topLeft.x = x + dx * cosRotation - dy * sinRotation;
                topLeft.y = y + dx * sinRotation + dy * cosRotation;

                topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
                topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

                bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
                bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

                bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
                bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
            }

            ::rlSetTexture(texture.id);
            ::rlBegin(RL_QUADS);

            ::rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            ::rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

            // Top-left corner for texture and quad
            if (flipX) ::rlTexCoord2f((source.x + source.width) / width, source.y / height);
            else ::rlTexCoord2f(source.x / width, source.y / height);
            ::rlVertex2f(topLeft.x, topLeft.y);

            // Top-right corner for texture and quad
            if (flipX) ::rlTexCoord2f(source.x / width, source.y / height);
            else ::rlTexCoord2f((source.x + source.width) / width, source.y / height);
            ::rlVertex2f(topRight.x, topRight.y);

            // Bottom-right corner for texture and quad
            if (flipX) ::rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            else ::rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            ::rlVertex2f(bottomRight.x, bottomRight.y);

            // Bottom-left corner for texture and quad
            if (flipX) ::rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            else ::rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            ::rlVertex2f(bottomLeft.x, bottomLeft.y);

            ::rlEnd();
            ::rlSetTexture(0);
        }
    }

    // A modified version of DrawRectanglePro to flip the quad
    void DrawRectangleProFlipped(Rectangle rec, Vector2 origin, float rotation, Color color)
    {
        ::Vector2 topLeft = { 0 };
        ::Vector2 topRight = { 0 };
        ::Vector2 bottomLeft = { 0 };
        ::Vector2 bottomRight = { 0 };

        // Only calculate rotation if needed
        if (rotation == 0.0f)
        {
            float x = rec.x - origin.x;
            float y = rec.y - origin.y;
            topLeft = { x, y };
            topRight = { x + rec.width, y };
            bottomLeft = { x, y + rec.height };
            bottomRight = { x + rec.width, y + rec.height };
        }
        else
        {
            float sinRotation = sinf(rotation * DEG2RAD);
            float cosRotation = cosf(rotation * DEG2RAD);
            float x = rec.x;
            float y = rec.y;
            float dx = -origin.x;
            float dy = -origin.y;

            topLeft.x = x + dx * cosRotation - dy * sinRotation;
            topLeft.y = y + dx * sinRotation + dy * cosRotation;

            topRight.x = x + (dx + rec.width) * cosRotation - dy * sinRotation;
            topRight.y = y + (dx + rec.width) * sinRotation + dy * cosRotation;

            bottomLeft.x = x + dx * cosRotation - (dy + rec.height) * sinRotation;
            bottomLeft.y = y + dx * sinRotation + (dy + rec.height) * cosRotation;

            bottomRight.x = x + (dx + rec.width) * cosRotation - (dy + rec.height) * sinRotation;
            bottomRight.y = y + (dx + rec.width) * sinRotation + (dy + rec.height) * cosRotation;
        }

#if defined(SUPPORT_QUADS_DRAW_MODE)
        ::rlSetTexture(::GetShapesTexture().id);
        ::Rectangle shapeRect = ::GetShapesTextureRectangle();

        ::rlBegin(RL_QUADS);

        ::rlNormal3f(0.0f, 0.0f, 1.0f);
        ::rlColor4ub(color.r, color.g, color.b, color.a);

        ::rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
        ::rlVertex2f(topLeft.x, topLeft.y);

        ::rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
        ::rlVertex2f(bottomLeft.x, bottomLeft.y);

        ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
        ::rlVertex2f(bottomRight.x, bottomRight.y);

        ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
        ::rlVertex2f(topRight.x, topRight.y);

        ::rlEnd();

        ::rlSetTexture(0);
#else
        ::rlBegin(RL_TRIANGLES);

        ::rlColor4ub(color.r, color.g, color.b, color.a);

        ::rlVertex2f(topLeft.x, topLeft.y);
        ::rlVertex2f(topRight.x, topRight.y);
        ::rlVertex2f(bottomLeft.x, bottomLeft.y);

        ::rlVertex2f(bottomLeft.x, bottomLeft.y);
        ::rlVertex2f(topRight.x, topRight.y);
        ::rlVertex2f(bottomRight.x, bottomRight.y);

        ::rlEnd();
#endif
    }

    void DrawRectangleOutline(Rectangle rec, Vector2 origin, float rotation, float lineThick, Color color)
    {
        if ((lineThick > rec.width) || (lineThick > rec.height))
        {
            if (rec.width > rec.height) lineThick = rec.height / 2;
            else if (rec.width < rec.height) lineThick = rec.width / 2;
        }

        Rectangle top = { rec.x, rec.y, rec.width, lineThick };
        Rectangle bottom = { rec.x, rec.y - lineThick + rec.height, rec.width, lineThick };
        Rectangle left = { rec.x, rec.y + lineThick, lineThick, rec.height - lineThick * 2.0f };
        Rectangle right = { rec.x - lineThick + rec.width, rec.y + lineThick, lineThick, rec.height - lineThick * 2.0f };

        DrawRectangleProFlipped(top, origin, rotation, { color.r, color.g, color.b, color.a });
        DrawRectangleProFlipped(bottom, origin, rotation, { color.r, color.g, color.b, color.a });
        DrawRectangleProFlipped(left, origin, rotation, { color.r, color.g, color.b, color.a });
        DrawRectangleProFlipped(right, origin, rotation, { color.r, color.g, color.b, color.a });
    }

    // A modified version of DrawCircleSector to flip the quad
    void DrawCircleSectorFlipped(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
    {
        if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

        // Function expects (endAngle > startAngle)
        if (endAngle < startAngle)
        {
            // Swap values
            float tmp = startAngle;
            startAngle = endAngle;
            endAngle = tmp;
        }

        int minSegments = (int)ceilf((endAngle - startAngle) / 90);

        if (segments < minSegments)
        {
            // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
            float th = acosf(2 * powf(1 - SMOOTH_CIRCLE_ERROR_RATE / radius, 2) - 1);
            segments = (int)((endAngle - startAngle) * ceilf(2 * PI / th) / 360);

            if (segments <= 0) segments = minSegments;
        }

        float stepLength = (endAngle - startAngle) / (float)segments;
        float angle = startAngle;

#if defined(SUPPORT_QUADS_DRAW_MODE)
        ::rlSetTexture(GetShapesTexture().id);
        ::Rectangle shapeRect = GetShapesTextureRectangle();

        ::rlBegin(RL_QUADS);

        // NOTE: Every QUAD actually represents two segments
        for (int i = 0; i < segments / 2; i++)
        {
            ::rlColor4ub(color.r, color.g, color.b, color.a);

            ::rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
            ::rlVertex2f(center.x, center.y);

            ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
            ::rlVertex2f(center.x + cosf(DEG2RAD * (angle + stepLength * 2.0f)) * radius, center.y + sinf(DEG2RAD * (angle + stepLength * 2.0f)) * radius);

            ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
            ::rlVertex2f(center.x + cosf(DEG2RAD * (angle + stepLength)) * radius, center.y + sinf(DEG2RAD * (angle + stepLength)) * radius);

            ::rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
            ::rlVertex2f(center.x + cosf(DEG2RAD * angle) * radius, center.y + sinf(DEG2RAD * angle) * radius);

            angle += (stepLength * 2.0f);
        }

        // NOTE: In case number of segments is odd, we add one last piece to the cake
        if (((unsigned int)segments % 2) == 1)
        {
            ::rlColor4ub(color.r, color.g, color.b, color.a);

            ::rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
            ::rlVertex2f(center.x, center.y);

            ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
            ::rlVertex2f(center.x + cosf(DEG2RAD * (angle + stepLength)) * radius, center.y + sinf(DEG2RAD * (angle + stepLength)) * radius);

            ::rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
            ::rlVertex2f(center.x + cosf(DEG2RAD * angle) * radius, center.y + sinf(DEG2RAD * angle) * radius);

            ::rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
            ::rlVertex2f(center.x, center.y);
        }

        ::rlEnd();

        ::rlSetTexture(0);
#else
        ::rlBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            ::rlColor4ub(color.r, color.g, color.b, color.a);

            ::rlVertex2f(center.x, center.y);
            ::rlVertex2f(center.x + cosf(DEG2RAD * angle) * radius, center.y + sinf(DEG2RAD * angle) * radius);
            ::rlVertex2f(center.x + cosf(DEG2RAD * (angle + stepLength)) * radius, center.y + sinf(DEG2RAD * (angle + stepLength)) * radius);

            angle += stepLength;
        }
        ::rlEnd();
#endif
    }

    void DrawCircleLinesV(Vector2 center, float radius, Color color)
    {
        rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i = 0; i < 360; i += 10)
        {
            rlVertex2f(center.x + cosf(DEG2RAD * i) * radius, center.y + sinf(DEG2RAD * i) * radius);
            rlVertex2f(center.x + cosf(DEG2RAD * (i + 10)) * radius, center.y + sinf(DEG2RAD * (i + 10)) * radius);
        }
        rlEnd();
    }


    // Input-related functions: keyboard
    bool IsKeyPressed(int key) {
        return ::IsKeyPressed(key);
    }

    bool IsKeyDown(int key) {
        return ::IsKeyDown(key);
    }

    bool IsKeyReleased(int key) {
        return ::IsKeyReleased(key);
    }

    bool IsKeyUp(int key) {
        return ::IsKeyUp(key);
    }

    void SetExitKey(int key) {
        ::SetExitKey(key);
    }

    int GetKeyPressed(void) {
        return ::GetKeyPressed();
    }

    int GetCharPressed(void) {
        return ::GetCharPressed();
    }

    // Input-related functions: gamepads
    bool IsGamepadAvailable(int gamepad) {
        return ::IsGamepadAvailable(gamepad);
    }

    const char* GetGamepadName(int gamepad) {
        return ::GetGamepadName(gamepad);
    }

    bool IsGamepadButtonPressed(int gamepad, int button) {
        return ::IsGamepadButtonPressed(gamepad, button);
    }

    bool IsGamepadButtonDown(int gamepad, int button) {
        return ::IsGamepadButtonDown(gamepad, button);
    }

    bool IsGamepadButtonReleased(int gamepad, int button) {
        return ::IsGamepadButtonReleased(gamepad, button);
    }

    bool IsGamepadButtonUp(int gamepad, int button) {
        return ::IsGamepadButtonUp(gamepad, button);
    }

    int GetGamepadButtonPressed(void) {
        return ::GetGamepadButtonPressed();
    }

    int GetGamepadAxisCount(int gamepad) {
        return ::GetGamepadAxisCount(gamepad);
    }

    float GetGamepadAxisMovement(int gamepad, int axis) {
        return ::GetGamepadAxisMovement(gamepad, axis);
    }

    int SetGamepadMappings(const char* mappings) {
        return ::SetGamepadMappings(mappings);
    }

    // Input-related functions: mouse
    bool IsMouseButtonPressed(int button) {
        return ::IsMouseButtonPressed(button);
    }

    bool IsMouseButtonDown(int button) {
        return ::IsMouseButtonDown(button);
    }

    bool IsMouseButtonReleased(int button) {
        return ::IsMouseButtonReleased(button);
    }

    bool IsMouseButtonUp(int button) {
        return ::IsMouseButtonUp(button);
    }

    int GetMouseX(void) {
        return ::GetMouseX();
    }

    int GetMouseY(void) {
        return ::GetMouseY();
    }

    Vector2 GetMousePosition(void) {
        ::Vector2 pos = ::GetMousePosition();
        return {pos.x, pos.y};
    }

    Vector2 GetMouseDelta(void) {
        ::Vector2 pos = ::GetMouseDelta();
        return { pos.x, pos.y };
    }

    void SetMousePosition(int x, int y) {
        ::SetMousePosition(x, y);
    }

    void SetMouseOffset(int offsetX, int offsetY) {
        ::SetMouseOffset(offsetX, offsetY);
    }

    void SetMouseScale(float scaleX, float scaleY) {
        ::SetMouseScale(scaleX, scaleY);
    }

    float GetMouseWheelMove(void) {
        return ::GetMouseWheelMove();
    }

    Vector2 GetMouseWheelMoveV(void) {
        ::Vector2 pos = ::GetMouseWheelMoveV();
        return { pos.x, pos.y };
    }

    void SetMouseCursor(int cursor) {
        ::SetMouseCursor(cursor);
    }

    // Input-related functions: touch
    int GetTouchX(void) {
        return ::GetTouchX();
    }

    int GetTouchY(void) {
        return ::GetTouchY();
    }

    Vector2 GetTouchPosition(int index) {
        ::Vector2 pos = ::GetTouchPosition(index);
        return { pos.x, pos.y };
    }

    int GetTouchPointId(int index) {
        return ::GetTouchPointId(index);
    }

    int GetTouchPointCount(void) {
        return ::GetTouchPointCount();
    }


    // Camera management functions
    void UpdateCamera(Camera* camera, int mode) {
        ::Camera camera2 = { {camera->position.x, camera->position.y, camera->position.z}, {camera->target.x, camera->target.y, camera->target.z}, {camera->up.x, camera->up.y, camera->up.z}, camera->fovy, camera->projection };
        ::UpdateCamera(&camera2, mode);
        camera->position.x = camera2.position.x;
        camera->position.y = camera2.position.y;
        camera->position.z = camera2.position.z;
        camera->target.x = camera2.target.x;
        camera->target.y = camera2.target.y;
        camera->target.z = camera2.target.z;
        camera->up.x = camera2.up.x;
        camera->up.y = camera2.up.y;
        camera->up.z = camera2.up.z;
        camera->fovy = camera2.fovy;
        camera->projection = camera2.projection;
    }

    //void UpdateCameraPro(Camera* camera, Vector3 movement, Vector3 rotation, float zoom) {
    //    ::UpdateCameraPro(camera, movement, rotation, zoom);
    //}


    // Filesystem-related functions
    //void UnloadDirectoryFiles(FilePathList files) {
    //    for (auto const& file : files) {
    //        ::UnloadFile(file.c_str());
    //    }
    //}

    bool IsFileDropped(void) {
        return ::IsFileDropped();
    }

    FilePathList LoadDroppedFiles(void) {
        ::FilePathList filePathList = ::LoadDroppedFiles();
        return { filePathList.capacity, filePathList.count, filePathList.paths };
    }

    void UnloadDroppedFiles(FilePathList files) {
        ::UnloadDroppedFiles({ files.capacity, files.count, files.paths });
    }



    void DrawGrid(int slices, float spacing) {
        ::DrawGrid(slices, spacing);
    }


    // Basic 2D Collisions
    bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2) {
        return ::CheckCollisionRecs({rec1.x, rec1.y, rec1.width, rec1.height}, { rec2.x, rec2.y, rec2.width, rec2.height });
    }

    bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2) {
        return ::CheckCollisionCircles({ center1.x, center1.y }, radius1, { center2.x, center2.y }, radius2);
    }

    bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
        return ::CheckCollisionCircleRec({center.x, center.y}, radius, { rec.x, rec.y, rec.width, rec.height });
    }

    // Basic 3D Collisions
    RayCollision GetRayCollisionBox(Ray ray, BoundingBox box)
    {
        ::RayCollision rayCollision = ::GetRayCollisionBox(
            { {ray.position.x, ray.position.y, ray.position.z}, { ray.direction.x, ray.direction.y, ray.direction.z } },
            { { box.min.x, box.min.y, box.min.z }, { box.max.x, box.max.y, box.max.z } });

        return { rayCollision.hit, rayCollision.distance, { rayCollision.point.x, rayCollision.point.y, rayCollision.point.z }, { rayCollision.normal.x, rayCollision.normal.y, rayCollision.normal.z } };
    }


    //Vector Math
    Vector2 Vector2Add(Vector2 v1, Vector2 v2)
    {
        Vector2 result = { v1.x + v2.x, v1.y + v2.y };

        return result;
    }

    Vector2 Vector2Subtract(Vector2 v1, Vector2 v2)
    {
        Vector2 result = { v1.x - v2.x, v1.y - v2.y };

        return result;
    }

    Vector3 Vector3Add(Vector3 v1, Vector3 v2)
    {
        Vector3 result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };

        return result;
    }

    Vector3 Vector3Subtract(Vector3 v1, Vector3 v2)
    {
        Vector3 result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };

        return result;
    }

    Vector3 Vector3Scale(Vector3 v, float scalar)
    {
        Vector3 result = { v.x * scalar, v.y * scalar, v.z * scalar };

        return result;
    }

    float Vector3Distance(Vector3 v1, Vector3 v2) {
        return ::Vector3Distance({ v1.x, v1.y, v1.z }, {v2.x, v2.y, v2.z});
    }

    Quaternion QuaternionFromEuler(float pitch, float yaw, float roll)
    {
        Quaternion result = { 0 };

        float x0 = cosf(pitch * 0.5f);
        float x1 = sinf(pitch * 0.5f);
        float y0 = cosf(yaw * 0.5f);
        float y1 = sinf(yaw * 0.5f);
        float z0 = cosf(roll * 0.5f);
        float z1 = sinf(roll * 0.5f);

        result.x = x1 * y0 * z0 - x0 * y1 * z1;
        result.y = x0 * y1 * z0 + x1 * y0 * z1;
        result.z = x0 * y0 * z1 - x1 * y1 * z0;
        result.w = x0 * y0 * z0 + x1 * y1 * z1;

        return result;
    }

    Vector3 QuaternionToEuler(Quaternion q)
    {
        Vector3 result = { 0 };

        // Roll (x-axis rotation)
        float x0 = 2.0f * (q.w * q.x + q.y * q.z);
        float x1 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
        result.x = atan2f(x0, x1);

        // Pitch (y-axis rotation)
        float y0 = 2.0f * (q.w * q.y - q.z * q.x);
        y0 = y0 > 1.0f ? 1.0f : y0;
        y0 = y0 < -1.0f ? -1.0f : y0;
        result.y = asinf(y0);

        // Yaw (z-axis rotation)
        float z0 = 2.0f * (q.w * q.z + q.x * q.y);
        float z1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
        result.z = atan2f(z0, z1);

        return result;
    }


    // Model animations
    //ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount) {
    //    ::ModelAnimation* raylibModelAnimations = ::LoadModelAnimations(fileName, animCount);

    //    ModelAnimation* wrapperModelAnimations = new ModelAnimation();

    //    wrapperModelAnimations->boneCount = raylibModelAnimations->boneCount;
    //    wrapperModelAnimations->frameCount = raylibModelAnimations->frameCount;

    //    wrapperModelAnimations->bones = new BoneInfo[raylibModelAnimations->boneCount];
    //    for (int i = 0; i < raylibModelAnimations->boneCount; ++i)
    //    {
    //        strcpy_s(wrapperModelAnimations->bones[i].name, sizeof(wrapperModelAnimations->bones[i].name), raylibModelAnimations->bones[i].name);
    //        wrapperModelAnimations->bones[i].parent = raylibModelAnimations->bones[i].parent;
    //    }

    //    wrapperModelAnimations->framePoses = new Transform * [raylibModelAnimations->frameCount];
    //    for (int i = 0; i < raylibModelAnimations->frameCount; ++i)
    //    {
    //        wrapperModelAnimations->framePoses[i] = new Transform[raylibModelAnimations->boneCount];
    //        for (int j = 0; j < raylibModelAnimations->boneCount; ++j)
    //        {
    //            wrapperModelAnimations->framePoses[i][j].translation = { raylibModelAnimations->framePoses[i][j].translation.x,
    //                raylibModelAnimations->framePoses[i][j].translation.y,
    //                raylibModelAnimations->framePoses[i][j].translation.z };
    //            wrapperModelAnimations->framePoses[i][j].rotation = {raylibModelAnimations->framePoses[i][j].rotation.x,
    //                raylibModelAnimations->framePoses[i][j].rotation.y,
    //                raylibModelAnimations->framePoses[i][j].rotation.z,
    //                raylibModelAnimations->framePoses[i][j].rotation.w };
    //            wrapperModelAnimations->framePoses[i][j].scale = { raylibModelAnimations->framePoses[i][j].scale.x,
    //                raylibModelAnimations->framePoses[i][j].scale.y,
    //                raylibModelAnimations->framePoses[i][j].scale.z };

    //        }
    //    }

    //    ::UnloadModelAnimations(raylibModelAnimations, *animCount);

    //    return wrapperModelAnimations;
    //}
    //void UpdateModelAnimation(Model model, ModelAnimation anim, int frame) { // Moved to RaylibModelWrapper
    //}
    void UnloadModelAnimation(ModelAnimation anim) {
        for (int i = 0; i < anim.frameCount; i++) RL_FREE(anim.framePoses[i]);

        RL_FREE(anim.bones);
        RL_FREE(anim.framePoses);
    }
    void UnloadModelAnimations(ModelAnimation* animations, int animCount) {
        for (int i = 0; i < animCount; i++) UnloadModelAnimation(animations[i]);
        RL_FREE(animations);
    }
    //bool IsModelAnimationValid(Model model, ModelAnimation anim) { // Moved to RaylibModelWrapper
    //    return false;
    //}


    const char* GetWorkingDirectory(void) {
        return ::GetWorkingDirectory();
    }

    // Materials
    //Material LoadMaterial() {
    // // This function is not needed
    //}

    void UnloadMaterial(Material material) {
        // Not using Raylib's UnloadMaterial() since it will unload the textures

        delete[] material.maps;
    }

    // Audio

    void InitAudioDevice(void) {
        ::InitAudioDevice();
    }

    void CloseAudioDevice(void) {
        ::CloseAudioDevice();
    }

    bool IsAudioDeviceReady(void) {
        return ::IsAudioDeviceReady();
    }

    void SetMasterVolume(float volume) {
        ::SetMasterVolume(volume);
    }

    float GetMasterVolume(void) {
        return ::GetMasterVolume();
    }


    //Wave LoadWave(const char* fileName)
    //{
    //    ::Wave wave = ::LoadWave(fileName);
    //    return { wave.frameCount, wave.sampleRate, wave.sampleSize, wave.channels, wave.data };
    //}

    //bool IsWaveReady(Wave wave)
    //{
    //    return ::IsWaveReady({ wave.frameCount, wave.sampleRate, wave.sampleSize, wave.channels, wave.data });
    //}

    //Sound LoadSound(const char* fileName)
    //{
    //    ::Sound sound = ::LoadSound(fileName);
    //    return { { sound.stream.buffer, sound.stream.processor, sound.stream.sampleRate, sound.stream.sampleSize, sound.stream.channels }, sound.frameCount };
    //}


    //Sound LoadSoundFromWave(Wave wave)
    //{
    //    return ::LoadSoundFromWave({ wave.frameCount, wave.sampleRate, wave.sampleSize, wave.channels, wave.data });
    //}

    //bool IsSoundReady(Sound sound)
    //{
    //    return ::IsSoundReady(sound);
    //}

    //void UpdateSound(Sound sound, const void* data, int sampleCount)
    //{
    //    ::UpdateSound(sound, data, sampleCount);
    //}

    //void UnloadWave(Wave wave)
    //{
    //    ::UnloadWave(wave);
    //}

    //void UnloadSound(Sound sound)
    //{
    //    ::UnloadSound(sound);
    //}

    //void PlaySound(Sound sound)
    //{
    //    ::PlaySound(sound);
    //}

    //void StopSound(Sound sound)
    //{
    //    ::StopSound(sound);
    //}

    //void PauseSound(Sound sound)
    //{
    //    ::PauseSound(sound);
    //}

    //void ResumeSound(Sound sound)
    //{
    //    ::ResumeSound(sound);
    //}

    //bool IsSoundPlaying(Sound sound)
    //{
    //    return ::IsSoundPlaying(sound);
    //}

    //void SetSoundVolume(Sound sound, float volume)
    //{
    //    ::SetSoundVolume(sound, volume);
    //}

    //void SetSoundPitch(Sound sound, float pitch)
    //{
    //    ::SetSoundPitch(sound, pitch);
    //}

    //void SetSoundPan(Sound sound, float pan)
    //{
    //    ::SetSoundPan(sound, pan);
    //}

    //Music LoadMusicStream(const char* fileName)
    //{
    //    return ::LoadMusicStream(fileName);
    //}

    //Music LoadMusicStreamFromMemory(const char* fileType, const unsigned char* data, int dataSize)
    //{
    //    return ::LoadMusicStreamFromMemory(fileType, data, dataSize);
    //}

    //bool IsMusicReady(Music music)
    //{
    //    return ::IsMusicReady(music);
    //}

    //void UnloadMusicStream(Music music)
    //{
    //    ::UnloadMusicStream(music);
    //}

    //void PlayMusicStream(Music music)
    //{
    //    ::PlayMusicStream(music);
    //}

    //bool IsMusicStreamPlaying(Music music)
    //{
    //    return ::IsMusicStreamPlaying(music);
    //}

    //void UpdateMusicStream(Music music)
    //{
    //    ::UpdateMusicStream(music);
    //}

    //void StopMusicStream(Music music)
    //{
    //    ::StopMusicStream(music);
    //}

    //void PauseMusicStream(Music music)
    //{
    //    ::PauseMusicStream(music);
    //}

    //void ResumeMusicStream(Music music)
    //{
    //    ::ResumeMusicStream(music);
    //}

    //void SetMusicVolume(Music music, float volume)
    //{
    //    ::SetMusicVolume(music, volume);
    //}

    //void SetMusicPitch(Music music, float pitch)
    //{
    //    ::SetMusicPitch(music, pitch);
    //}

    //void SetMusicPan(Music music, float pan)
    //{
    //    ::SetMusicPan(music, pan);
    //}

    //float GetMusicTimeLength(Music music)
    //{
    //    return ::GetMusicTimeLength(music);
    //}

    //float GetMusicTimePlayed(Music music)
    //{
    //    return ::GetMusicTimePlayed(music);
    //}

    // Color functions
    Vector4 ColorNormalize(Color color) {
        ::Vector4 vector = ::ColorNormalize({ color.r, color.g, color.b, color.a });
        return { vector.x, vector.y, vector.z, vector.w };
    }

    // rlgl functions

    void rlSetMatrixProjection(Matrix matrix) {
        return ::rlSetMatrixProjection({ matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 });
    }

    Matrix rlGetMatrixModelview() {
        ::Matrix matrix = ::rlGetMatrixModelview();
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    Matrix rlGetMatrixProjection() {
        ::Matrix matrix = ::rlGetMatrixProjection();
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    void rlSetUniform(int locIndex, const void* value, int uniformType, int count) {
        ::rlSetUniform(locIndex, value, uniformType, count);
    }

    unsigned int rlLoadFramebuffer() {
        return ::rlLoadFramebuffer();
    }

    void rlUnloadFramebuffer(unsigned int id) {
        ::rlUnloadFramebuffer(id);
    }

    void rlEnableFramebuffer(unsigned int id) {
        ::rlEnableFramebuffer(id);
    }

    void rlDisableFramebuffer() {
        ::rlDisableFramebuffer();
    }

    void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel) {
        ::rlFramebufferAttach(fboId, texId, attachType, texType, mipLevel);
    }

    bool rlFramebufferComplete(unsigned int id) {
        return ::rlFramebufferComplete(id);
    }

    unsigned int rlLoadTexture(const void* data, int width, int height, int format, int mipmapCount) {
        return ::rlLoadTexture(data, width, height, format, mipmapCount);
    }

    unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer) {
        return ::rlLoadTextureDepth(width, height, useRenderBuffer);
    }

    void rlEnableTexture(unsigned int id) {
        ::rlEnableTexture(id);
    }

    void rlDisableTexture() {
        ::rlDisableTexture();
    }

    void rlEnableShader(unsigned int id) {
        ::rlEnableShader(id);
    }

    void rlDisableShader() {
        ::rlDisableShader();
    }

    void rlActiveTextureSlot(int slot) {
        ::rlActiveTextureSlot(slot);
    }

    void rlSetUniformSampler(int locIndex, unsigned int textureId) {
        ::rlSetUniformSampler(locIndex, textureId);
    }

    void rlSetUniformMatrix(int locIndex, Matrix matrix) {
        ::rlSetUniformMatrix(locIndex, { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 });
    }

    void rlClearScreenBuffers() {
        ::rlClearScreenBuffers();
    }

    void rlDisableColorBlend() {
        ::rlDisableColorBlend();
    }

    void rlBegin(int mode) {
        ::rlBegin(mode);
    }

    void rlEnd() {
        ::rlEnd();
    }

    void rlVertex3f(float x, float y, float z) {
        ::rlVertex3f(x, y, z);
    }

    void rlDrawRenderBatchActive() {
        ::rlDrawRenderBatchActive();
    }

    // raymath functions

    Matrix MatrixOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane) {
        ::Matrix matrix = ::MatrixOrtho(left, right, bottom, top, nearPlane, farPlane);
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    Matrix MatrixMultiply(Matrix left, Matrix right) {
        ::Matrix matrix = ::MatrixMultiply(
            { left.m0, left.m1, left.m2, left.m3, left.m4, left.m5, left.m6, left.m7, left.m8, left.m9, left.m10, left.m11, left.m12, left.m13, left.m14, left.m15 },
            { right.m0, right.m1, right.m2, right.m3, right.m4, right.m5, right.m6, right.m7, right.m8, right.m9, right.m10, right.m11, right.m12, right.m13, right.m14, right.m15 });
        return { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 };
    }

    Vector3 Vector3Normalize(Vector3 v) {
        ::Vector3 vector = ::Vector3Normalize({ v.x, v.y, v.z });
        return { vector.x, vector.y, vector.z };
    }

    Matrix MatrixRotateXYZ(Vector3 angle)
    {
        Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f };

        float cosz = cosf(-angle.z);
        float sinz = sinf(-angle.z);
        float cosy = cosf(-angle.y);
        float siny = sinf(-angle.y);
        float cosx = cosf(-angle.x);
        float sinx = sinf(-angle.x);

        result.m0 = cosz * cosy;
        result.m1 = (cosz * siny * sinx) - (sinz * cosx);
        result.m2 = (cosz * siny * cosx) + (sinz * sinx);

        result.m4 = sinz * cosy;
        result.m5 = (sinz * siny * sinx) + (cosz * cosx);
        result.m6 = (sinz * siny * cosx) - (cosz * sinx);

        result.m8 = -siny;
        result.m9 = cosy * sinx;
        result.m10 = cosy * cosx;

        return result;
    }

    Vector3 Vector3Transform(Vector3 v, Matrix matrix) {
        ::Vector3 result = ::Vector3Transform({ v.x, v.y, v.z },
            { matrix.m0, matrix.m1, matrix.m2, matrix.m3, matrix.m4, matrix.m5, matrix.m6, matrix.m7, matrix.m8, matrix.m9, matrix.m10, matrix.m11, matrix.m12, matrix.m13, matrix.m14, matrix.m15 });
        return { result.x, result.y, result.z };
    }

    Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2) {
        Vector3 result = { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
        return result;
    }

    // Models
    Model LoadModelFromMesh(Mesh mesh) { // This is unfinished as there is a lot needed to convert everything
        ::Model model;
        ::Matrix matrix = { mesh.boneMatrices->m0, mesh.boneMatrices->m1, mesh.boneMatrices->m2, mesh.boneMatrices->m3, mesh.boneMatrices->m4, mesh.boneMatrices->m5, mesh.boneMatrices->m6, mesh.boneMatrices->m7,
            mesh.boneMatrices->m8, mesh.boneMatrices->m9, mesh.boneMatrices->m10, mesh.boneMatrices->m11, mesh.boneMatrices->m12, mesh.boneMatrices->m13, mesh.boneMatrices->m14, mesh.boneMatrices->m15 };
        // Todo: The code above wont work. boneMatrices is a container

        model = ::LoadModelFromMesh({ mesh.vertexCount, mesh.triangleCount, mesh.vertices, mesh.texcoords, mesh.texcoords2, mesh.normals, mesh.tangents, mesh.colors, mesh.indices, mesh.animVertices,
            mesh.animNormals, mesh.boneIds, mesh.boneWeights, &matrix, mesh.boneCount, mesh.vaoId, mesh.vboId });

        Mesh* modelMeshes = (Mesh*)RL_MALLOC(model.meshCount * sizeof(Mesh));

        for (size_t i = 0; i < model.meshCount; i++)
        {
            ::Mesh& mesh = model.meshes[i];

            Matrix* boneMatrices = nullptr;
            if (mesh.boneCount > 0 && mesh.boneMatrices != nullptr)
            {
                boneMatrices = (Matrix*)RL_MALLOC(model.boneCount * sizeof(Matrix));

                memcpy(boneMatrices, mesh.boneMatrices, mesh.boneCount * sizeof(Matrix));

                //for (size_t j = 0; j < mesh.boneCount; j++) {
                //    boneMatrices[j] = { mesh.boneMatrices[j].m0, mesh.boneMatrices[j].m1, mesh.boneMatrices[j].m2, mesh.boneMatrices[j].m3, mesh.boneMatrices[j].m4, mesh.boneMatrices[j].m5,
                //        mesh.boneMatrices[j].m6, mesh.boneMatrices[j].m7, mesh.boneMatrices[j].m8, mesh.boneMatrices[j].m9, mesh.boneMatrices[j].m10, mesh.boneMatrices[j].m11,
                //        mesh.boneMatrices[j].m12, mesh.boneMatrices[j].m13, mesh.boneMatrices[j].m14, mesh.boneMatrices[j].m15 };
                //}
            }

            modelMeshes[i] = {
                mesh.vertexCount,
                mesh.triangleCount,
                mesh.vertices,
                mesh.texcoords,
                mesh.texcoords2,
                mesh.normals,
                mesh.tangents,
                mesh.colors,
                mesh.indices,
                mesh.animVertices,
                mesh.animNormals,
                mesh.boneIds,
                mesh.boneWeights,
                boneMatrices,
                mesh.boneCount,
                mesh.vaoId,
                mesh.vboId
            };
        }

        //return { { model.transform.m0, model.transform.m1, model.transform.m2, model.transform.m3, model.transform.m4, model.transform.m5, model.transform.m6, model.transform.m7, model.transform.m8, model.transform.m9,model.transform.m10, model.transform.m11, model.transform.m12, model.transform.m13, model.transform.m14, model.transform.m15 },
        //    model.meshCount, model.materialCount,
        //    modelMeshes,
        //    model.materials,
        //    model.meshMaterial, model.boneCount,
        //    model.bones,
        //    model.bindPose 
        //};

        return {};
    }

    void UploadMesh(Mesh* mesh, bool dynamic) {
		::UploadMesh((::Mesh*)mesh, dynamic);
    }

    void UnloadMesh(Mesh mesh) {
        ::UnloadMesh({ mesh.vertexCount, mesh.triangleCount, mesh.vertices, mesh.texcoords, mesh.texcoords2, mesh.normals, mesh.tangents, mesh.colors, mesh.indices, mesh.animVertices,
            mesh.animNormals, mesh.boneIds, mesh.boneWeights, (::Matrix*)mesh.boneMatrices, mesh.boneCount, mesh.vaoId, mesh.vboId});
    }

    // ImGui Raylib
    bool ImGui_ImplRaylib_Init() {
        return ::ImGui_ImplRaylib_Init();
    }

    void ImGui_ImplRaylib_Shutdown() {
        ::ImGui_ImplRaylib_Shutdown();
    }

    void ImGui_ImplRaylib_NewFrame() {
        ::ImGui_ImplRaylib_NewFrame();
    }

    void ImGui_ImplRaylib_RenderDrawData(ImDrawData* draw_data) {
        ::ImGui_ImplRaylib_RenderDrawData(draw_data);
    }

    bool ImGui_ImplRaylib_ProcessEvents() {
        return ::ImGui_ImplRaylib_ProcessEvents();
    }


    void rlImGuiSetup(bool darkTheme) {
        ::rlImGuiSetup(darkTheme);
    }

    void rlImGuiBegin() {
        ::rlImGuiBegin();
    }

    void rlImGuiEnd() {
        ::rlImGuiEnd();
    }

    void rlImGuiShutdown() {
        ::rlImGuiShutdown();
    }

    void rlImGuiBeginInitImGui() {
        ::rlImGuiBeginInitImGui();
    }

    void rlImGuiEndInitImGui() {
        ::rlImGuiEndInitImGui();
    }

    void rlImGuiReloadFonts() {
        ::rlImGuiReloadFonts();
    }

    void rlImGuiBeginDelta(float deltaTime) {
        ::rlImGuiBeginDelta(deltaTime);
    }

    void rlImGuiImage(const Texture* image) {
        ImGui::Image((ImTextureID)image, ImVec2(float(image->width), float(image->height)));
    }

    void rlImGuiImageSize(const Texture* image, int width, int height) {
        ImGui::Image((ImTextureID)image, ImVec2(float(width), float(height)));
    }

    void rlImGuiImageSizeV(const Texture* image, Vector2 size) {
        ImGui::Image((ImTextureID)image, ImVec2(size.x, size.y));
    }

    void rlImGuiImageSizeTintV(const Texture* image, Vector2 size, Vector4 tint)
    {
        ImGui::Image((ImTextureID)image, ImVec2(size.x, size.y), { 0,0 }, {1,1}, ImVec4(tint.x, tint.y, tint.z, tint.w));
    }

    void rlImGuiImageRect(const Texture* image, int destWidth, int destHeight, Rectangle sourceRect) {
        ImVec2 uv0;
        ImVec2 uv1;

        if (sourceRect.width < 0)
        {
            uv0.x = -((float)sourceRect.x / image->width);
            uv1.x = (uv0.x - (float)(fabs(sourceRect.width) / image->width));
        }
        else
        {
            uv0.x = (float)sourceRect.x / image->width;
            uv1.x = uv0.x + (float)(sourceRect.width / image->width);
        }

        if (sourceRect.height < 0)
        {
            uv0.y = -((float)sourceRect.y / image->height);
            uv1.y = (uv0.y - (float)(fabs(sourceRect.height) / image->height));
        }
        else
        {
            uv0.y = (float)sourceRect.y / image->height;
            uv1.y = uv0.y + (float)(sourceRect.height / image->height);
        }

        ImGui::Image((ImTextureID)image, ImVec2(float(destWidth), float(destHeight)), uv0, uv1);
    }

    void rlImGuiImageRenderTexture(const RenderTexture* image) {
        rlImGuiImageRect(&image->texture, image->texture.width, image->texture.height, Rectangle{ 0,0, float(image->texture.width), -float(image->texture.height) });
    }

    void rlImGuiImageRenderTextureFit(const RenderTexture* image, bool center) {
        ImVec2 area = ImGui::GetContentRegionAvail();

        float scale = area.x / image->texture.width;

        float y = image->texture.height * scale;
        if (y > area.y)
        {
            scale = area.y / image->texture.height;
        }

        int sizeX = int(image->texture.width * scale);
        int sizeY = int(image->texture.height * scale);

        if (center)
        {
            ImGui::SetCursorPosX(0);
            ImGui::SetCursorPosX(area.x / 2 - sizeX / 2);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (area.y / 2 - sizeY / 2));
        }

        rlImGuiImageRect(&image->texture, sizeX, sizeY, Rectangle{ 0,0, float(image->texture.width), -float(image->texture.height) });
    }

    bool rlImGuiImageButton(const char* name, const Texture* image) {
        return ImGui::ImageButton(name, (ImTextureID)image, ImVec2(float(image->width), float(image->height)));
    }

    bool rlImGuiImageButtonSize(const char* name, const Texture* image, ImVec2 size) {
        return ImGui::ImageButton(name, (ImTextureID)image, size);
    }

    bool rlImGuiImageButtonSizeTint(const char* name, const Texture* image, ImVec2 size, ImVec4 tint) {
        return ImGui::ImageButton(name, (ImTextureID)image, size, { 0,0 }, {1, 1}, { 0,0,0,0 }, tint);
    }
}