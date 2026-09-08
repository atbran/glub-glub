const fs = require('fs');
const path = require('path');
const { PNG } = require('pngjs');
const { GIFEncoder, quantize, applyPalette } = require('gifenc');

const dir = process.argv[2];
const out = process.argv[3];
const scale = parseFloat(process.argv[4] || '0.6');
const delayMs = parseInt(process.argv[5] || '110', 10);

const files = fs.readdirSync(dir).filter((f) => f.endsWith('.png')).sort();
if (files.length === 0) throw new Error('no frames found in ' + dir);

const enc = GIFEncoder();
let W = 0, H = 0;

for (const f of files)
{
    const png = PNG.sync.read(fs.readFileSync(path.join(dir, f)));
    W = Math.round(png.width * scale);
    H = Math.round(png.height * scale);
    const rgba = new Uint8Array(W * H * 4);
    for (let y = 0; y < H; ++y)
    {
        for (let x = 0; x < W; ++x)
        {
            const sx = Math.min(png.width - 1, Math.floor(x / scale));
            const sy = Math.min(png.height - 1, Math.floor(y / scale));
            const si = (sy * png.width + sx) * 4;
            const di = (y * W + x) * 4;
            rgba[di] = png.data[si];
            rgba[di + 1] = png.data[si + 1];
            rgba[di + 2] = png.data[si + 2];
            rgba[di + 3] = 255;
        }
    }
    const palette = quantize(rgba, 64);
    const index = applyPalette(rgba, palette);
    enc.writeFrame(index, W, H, { palette, delay: delayMs });
}

enc.finish();
fs.writeFileSync(out, enc.bytes());
console.log('wrote', out, W + 'x' + H, (enc.bytes().length / 1024 / 1024).toFixed(2) + ' MB,', files.length, 'frames');
