float halton(u32 i, u32 b)
{
    float f = 1;
    float r = 0;
    while (i > 0) {
        f = f / b;
        r += f * (i % b);
        i = floor(i / b);
    }
    return r;
}
