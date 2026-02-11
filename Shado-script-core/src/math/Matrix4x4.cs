using System;
using System.Runtime.InteropServices;

namespace Shado;

[StructLayout(LayoutKind.Explicit)]
public struct Matrix4x4
{
    [FieldOffset( 0)] public float D00;
    [FieldOffset( 4)] public float D10;
    [FieldOffset( 8)] public float D20;
    [FieldOffset(12)] public float D30;
    [FieldOffset(16)] public float D01;
    [FieldOffset(20)] public float D11;
    [FieldOffset(24)] public float D21;
    [FieldOffset(28)] public float D31;
    [FieldOffset(32)] public float D02;
    [FieldOffset(36)] public float D12;
    [FieldOffset(40)] public float D22;
    [FieldOffset(44)] public float D32;
    [FieldOffset(48)] public float D03;
    [FieldOffset(52)] public float D13;
    [FieldOffset(56)] public float D23;
    [FieldOffset(60)] public float D33;

    public Matrix4x4 Inverse()
    {
        unsafe
        {
            return InternalCalls.Math_InverseMatrix(this);
        }
    }

    public override string ToString()
    {
        return $"{D00:0.00}  {D10:0.00}  {D20:0.00}  {D30:0.00}\n" +
            $"{D01:0.00}  {D11:0.00}  {D21:0.00}  {D31:0.00}\n" +
            $"{D02:0.00}  {D12:0.00}  {D22:0.00}  {D32:0.00}\n" +
            $"{D03:0.00}  {D13:0.00}  {D23:0.00}  {D33:0.00}";
    }

    // * operator
    public static Matrix4x4 operator *(Matrix4x4 a, Matrix4x4 b)
    {
        unsafe { return InternalCalls.Math_MultiplyMatrix(a, b); }
    }
    
    public static Vector4 operator *(Matrix4x4 a, Vector4 b)
    {
        unsafe { return InternalCalls.Math_MultiplyMatrixVector4(a, b); }
    }
}
