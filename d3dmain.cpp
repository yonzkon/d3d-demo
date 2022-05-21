#include "d3dUtility.h"

// Represent graphics card, so the global one is enough
IDirect3DDevice9 *device = 0;
const int deviceWidth = 1024;
const int deviceHeight = 768;
IDirect3DVertexBuffer9 *vbuf = NULL;
IDirect3DIndexBuffer9 *ibuf = NULL;


struct Vertex {
    Vertex(float x, float y, float z)
    {
        _x = x;
        _y = y;
        _z = z;
    }
    float _x, _y, _z;
    static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ;

bool Setup()
{
    device->CreateVertexBuffer(
        8 * sizeof(Vertex),
        D3DUSAGE_WRITEONLY,
        Vertex::FVF,
        D3DPOOL_MANAGED,
        &vbuf,
        0
    );

    device->CreateIndexBuffer(
        36 * sizeof(WORD),
        D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &ibuf,
        0
    );

    Vertex *vts;
    vbuf->Lock(0, 0, (void **)&vts, 0);
    vts[0] = Vertex(-1.0f, -1.0f, -1.0f);
    vts[1] = Vertex(-1.0f, 1.0f, -1.0f);
    vts[2] = Vertex(1.0f, 1.0f, -1.0f);
    vts[3] = Vertex(1.0f, -1.0f, -1.0f);
    vts[4] = Vertex(-1.0f, -1.0f, 1.0f);
    vts[5] = Vertex(-1.0f, 1.0f, 1.0f);
    vts[6] = Vertex(1.0f, 1.0f, 1.0f);
    vts[7] = Vertex(1.0f, -1.0f, 1.0f);
    vbuf->Unlock();

    WORD *ids = NULL;
    ibuf->Lock(0, 0, (void **)&ids, 0);
    // front side
    ids[0]  = 0; ids[1]  = 1; ids[2]  = 2;
    ids[3]  = 0; ids[4]  = 2; ids[5]  = 3;
    // back side
    ids[6]  = 4; ids[7]  = 6; ids[8]  = 5;
    ids[9]  = 4; ids[10] = 7; ids[11] = 6;
    // left side
    ids[12] = 4; ids[13] = 5; ids[14] = 1;
    ids[15] = 4; ids[16] = 1; ids[17] = 0;
    // right side
    ids[18] = 3; ids[19] = 2; ids[20] = 6;
    ids[21] = 3; ids[22] = 6; ids[23] = 7;
    // top
    ids[24] = 1; ids[25] = 5; ids[26] = 6;
    ids[27] = 1; ids[28] = 6; ids[29] = 2;
    // bottom
    ids[30] = 4; ids[31] = 0; ids[32] = 3;
    ids[33] = 4; ids[34] = 3; ids[35] = 7;
    ibuf->Unlock();

    // set projection
    D3DXMATRIX prj;
    D3DXMatrixPerspectiveFovLH(
        &prj,
        D3DX_PI * 0.5f,
        (float)deviceWidth / (float)deviceHeight,
        1.0f,
        1000.0f
    );
    device->SetTransform(D3DTS_PROJECTION, &prj);

    // set render state
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    return true;
}

void Cleanup()
{
    vbuf->Release();
    ibuf->Release();
}

bool Display(float timeDelta)
{
    if (device) {
        // set viewport
        static float ax = 5.0f;
        static float ay = 0.0f;
        static float directx = 1.0f;
        static float directy = 1.0f;
        ax += timeDelta * directx;
        ay += timeDelta * directy;
        if (ax > 8.0f)
            directx = -1.0f;
        else if (ax < -8.0f)
            directx = 1.0f;
        if (ay > 5.0f)
            directy = -1.0f;
        else if (ay < -5.0f)
            directy = 1.0f;
        D3DXVECTOR3 eye(0.0f, 0.0f, -5.0f);
        D3DXVECTOR3 at(ax, ay, 0.0f);
        D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
        D3DXMATRIX view;
        D3DXMatrixLookAtLH(&view, &eye, &at, &up);
        device->SetTransform(D3DTS_VIEW, &view);

        // set model
        D3DXMATRIX rx, ry;
        D3DXMatrixRotationX(&rx, 3.14f / 4.0f);
        static float y = 0.0f;
        D3DXMatrixRotationY(&ry, y);
        y += timeDelta;
        if (y >= 6.28f)
            y = 0.0f;
        D3DXMATRIX p = rx * ry;
        device->SetTransform(D3DTS_WORLD, &p);

        // Instruct the device to set each pixel on the back buffer black -
        // D3DCLEAR_TARGET: 0x00000000 (black) - and to set each pixel on
        // the depth buffer to a value of 1.0 - D3DCLEAR_ZBUFFER: 1.0f.
        device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
        device->BeginScene();

        device->SetStreamSource(0, vbuf, 0, sizeof(Vertex));
        device->SetIndices(ibuf);
        device->SetFVF(Vertex::FVF);

        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

        // Swap the back and front buffers.
        device->EndScene();
        device->Present(0, 0, 0, 0);
    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            DestroyWindow(hwnd);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI 
WinMain(HINSTANCE hinstance, HINSTANCE prevInstance,  PSTR cmdLine, int showCmd)
{
    if (!d3d::InitD3D(hinstance, deviceWidth, deviceHeight, true, D3DDEVTYPE_HAL, &device)) {
        MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }
        
    if (!Setup()) {
        MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }

    d3d::EnterMsgLoop(Display);

    Cleanup();
    device->Release();
    return 0;
}