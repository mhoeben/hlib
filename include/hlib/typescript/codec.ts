//
// MIT License
//
// Copyright (c) 2023 Maarten Hoeben
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
export namespace hlib_codec
{

export let config =
{
    binary: {
        encoder: {
            grow: 4096
        },
        decoder: {
        }
    }
};

export interface Encoder
{
    openStruct(name: string, value: number): void;
    openArray(name: string, value: number): void;
    openMap(name: string, value: number): void;
    transformBool(name: string, value: boolean): void;
    transformInt(name: string, value: number): void;
    transformFloat32(name: string, value: number): void;
    transformFloat64(name: string, value: number): void;
    transformString(name: string, value: string): void;
    transformBlob(name: string, value: Uint8Array): void;
    close(): void;
};

export interface Decoder
{ 
    openStruct(name: string): number;
    openArray(name: string): number;
    openMap(name: string): number;
    transformBool(name: string): boolean;
    transformInt(name: string): number;
    transformFloat32(name: string): number;
    transformFloat64(name: string): number;
    transformString(name: string): string;
    transformBlob(name: string): Uint8Array;
    close(): void;
}

class ProducerUint8Array
{
    //
    // Public
    //
    constructor(grow_size: number)
    {
        this.grow_size = grow_size;
        this.data = new Uint8Array(this.grow_size);
        this.view = new DataView(this.data.buffer);
        this.size = 0;
    }

    clear(): void
    {
        this.size = 0;
    }

    produceByte(value: number): void
    {
        this.reserve(1);
        this.view.setInt8(this.size, value);
        this.size += 1;
    }

    produceFloat32(value: number): void
    {
        this.reserve(4);
        this.view.setFloat32(this.size, value);
        this.size += 4;
    }

    produceFloat64(value: number): void
    {
        this.reserve(8);
        this.view.setFloat64(this.size, value);
        this.size += 8;
    }

    produceUint8Array(value: Uint8Array): void
    {
        this.reserve(value.byteLength);
        this.data.set(value, this.size);
        this.size += value.byteLength;
    }

    produceUint8ArraySubstr(value: Uint8Array, begin?: number, end?: number)
    {
        const subarray = value.subarray(begin, end);

        this.reserve(subarray.byteLength);
        this.data.set(subarray, this.size);
        this.size += subarray.byteLength;
    }

    produceString(value: string): void
    {
        this.produceUint8Array(new TextEncoder().encode(value));
    }

    //
    // Implementation
    //
    private readonly grow_size: number;

    private data : Uint8Array;
    private view : DataView;
    private size : number;

    reserve(extra: number): void
    {
        if (undefined === this.data) {
            this.data = new Uint8Array(this.grow_size);
        }
        else if (this.size + extra > this.view.byteLength) {
            const old_data: Uint8Array = this.data;

            this.data =  new Uint8Array(Math.max(this.size + extra, this.view.byteLength + this.grow_size));
			this.data.set(new Uint8Array(old_data.buffer.slice(0, this.size)));
			this.view = new DataView(this.data.buffer);
		}
    }
};

class ConsumerUint8Array
{
    constructor(buffer: ArrayBuffer)
    {
        this.view = new DataView(buffer);
        this.offset = 0;
    }

    size(): number
    {
        return this.view.byteLength;
    }

    remaining(): number
    {
        console.assert(this.offset <= this.size());
        return this.size() - this.offset;
    }

    consumeByte(): number
    {
        return this.view.getUint8(this.consume(1));
    }

    consumeFloat32(): number
    {
        return this.view.getFloat32(this.consume(4));
    }

    consumeFloat64(): number
    {
        return this.view.getFloat32(this.consume(8));
    }

    consumeUint8Array(size: number): Uint8Array
    {
        return new Uint8Array(this.view.buffer, this.consume(size), size);
    }

    consumeString(size: number): string
    {
        return new TextDecoder().decode(this.consumeUint8Array(size));
    }

    //
    // Implementation
    //
    private view: DataView;
    private offset: number;

    private consume(size: number): number
    {
        const offset: number = this.offset;
        this.offset += size;
        return offset;
    }
};

class BinaryEncoder implements Encoder
{
    //
    // Public
    //
    constructor()
    {
        this.array = new ProducerUint8Array(config.binary.encoder.grow);
    }
    
    openStruct(_name: string, _value: number): void
    {
    }

    openArray(name: string, value: number): void
    {
        this.transformInt(name, value);
    }

    openMap(name: string, value: number): void
    {
        this.transformInt(name, value);
    }

    transformBool(_name: string, value: boolean): void
    {
        this.array.produceByte(Number(!!value));
    }

    transformInt(_name: string, value: number): void
    {
        const data: Uint8Array = new Uint8Array(8);
        let size: number = 0;
        
        let negative: number;
        if (value < 0) {
            value = -value;
            negative = 0x40;
        } else {
            negative = 0x00;
        }
        
        const bits: number = 32 - Math.clz32(value);
        
        data[size++] = negative | (value & 0x3f);
        if (bits > 6) {
            data[size - 1] |= 0x80;
            value >>= 6;
        
            for (let i = 6; i < bits; i += 7) {
                data[size++] = 0x80 | (value & 0x7f);
                value >>= 7;
            }
        
            data[size - 1] &= ~0x80;
        }
    
        this.array.produceUint8ArraySubstr(data, 0, size);
    }

    transformFloat32(_name: string, value: number): void
    {
        this.array.produceFloat32(value);
    }

    transformFloat64(_name: string, value: number): void
    {
        this.array.produceFloat32(value);
    }

    transformString(_name: string, value: string): void
    {
        const utf8: Uint8Array = new TextEncoder().encode(value);

        this.transformInt(null, utf8.byteLength);
        this.array.produceUint8Array(utf8);
    }

    transformBlob(_name: string, value: Uint8Array): void
    {
        this.transformInt(null, value.byteLength);
        this.array.produceUint8Array(value);
    }

    close(): void
    {
    }

    //
    // Implementation
    //
    array: ProducerUint8Array;
};

class BinaryDecoder implements Decoder
{
    constructor(buffer: ArrayBuffer)
    {
        this.array = new ConsumerUint8Array(buffer);
    }

    openStruct(name: string): number
    {
        return 0;
    }

    openArray(name: string): number
    {
        return this.transformInt(name);
    }

    openMap(name: string): number
    {
        return this.transformInt(name);
    }

    transformBool(_name: string): boolean
    {
        return Boolean(this.array.consumeByte());
    }

    transformInt(_name: string): number
    {
        let byte = this.array.consumeByte();

        let value: number = byte & 0x3f;
        let negative: boolean = Boolean(byte ^ 0x40);
        let shift = 6;

        while (byte & 0x80) {
            byte = this.array.consumeByte();
            value |= (byte & 0x7f) << shift;
            shift += 7;
        }

        return negative ? -value : value;
    }

    transformFloat32(_name: string): number
    {
        return this.array.consumeFloat32();
    }

    transformFloat64(_name: string): number
    {
        return this.array.consumeFloat64();
    }

    transformString(_name: string): string
    {
        const length: number = this.transformInt(null);
        const utf8: Uint8Array = this.array.consumeUint8Array(length);
        return new TextDecoder().decode(utf8);
    }

    transformBlob(_name: string): Uint8Array
    {
        const size: number = this.transformInt(null);
        return this.array.consumeUint8Array(size);
    }

    close(): void
    {
    }

    //
    // Implementation
    //
    array: ConsumerUint8Array;
};

} // namespace hlib_codec

