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

const config =
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
};

export interface Type
{
    readonly __id : number;
    readonly __size : number;

    encode(encoder: Encoder): void;
    decode(decoder: Decoder): void;
};

export interface Encoder
{
    openType(name: string, value: Type): void;
    openArray(name: string, value: number): void;
    openMap(name: string, value: number): void;
    encodeBool(name: string, value: boolean): void;
    encodeInt(name: string, value: number): void;
    encodeFloat32(name: string, value: number): void;
    encodeFloat64(name: string, value: number): void;
    encodeString(name: string, value: string): void;
    encodeBinary(name: string, value: Uint8Array): void;
    close(): void;

    wrap(object: Type): void;
};

export interface Decoder
{ 
    openType(name: string): number;
    openArray(name: string): number;
    openMap(name: string): number;
    decodeBool(name: string): boolean;
    decodeInt(name: string): number;
    decodeFloat32(name: string): number;
    decodeFloat64(name: string): number;
    decodeString(name: string): string;
    decodeBinary(name: string): Uint8Array;
    close(): void;

    peek(): number;
    unwrap(object: Type): void;
};

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

    mark(): number
    {
        return this.offset;
    }

    restore(offset: number)
    {
        this.offset = offset;
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
    
    openType(_name: string, _value: Type): void
    {
    }

    openArray(name: string, value: number): void
    {
        this.encodeInt(name, value);
    }

    openMap(name: string, value: number): void
    {
        this.encodeInt(name, value);
    }

    encodeBool(_name: string, value: boolean): void
    {
        this.array.produceByte(Number(!!value));
    }

    encodeInt(_name: string, value: number): void
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

    encodeFloat32(_name: string, value: number): void
    {
        this.array.produceFloat32(value);
    }

    encodeFloat64(_name: string, value: number): void
    {
        this.array.produceFloat32(value);
    }

    encodeString(_name: string, value: string): void
    {
        const utf8: Uint8Array = new TextEncoder().encode(value);

        this.encodeInt(null, utf8.byteLength);
        this.array.produceUint8Array(utf8);
    }

    encodeBinary(_name: string, value: Uint8Array): void
    {
        this.encodeInt(null, value.byteLength);
        this.array.produceUint8Array(value);
    }

    close(): void
    {
    }

    wrap(type: Type): void
    {
        this.openArray(null, 2);
        this.encodeInt(null, type.__id);
        type.encode(this);
        this.close();
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

    openType(_name: string): number
    {
        return 0;
    }

    openArray(name: string): number
    {
        return this.decodeInt(name);
    }

    openMap(name: string): number
    {
        return this.decodeInt(name);
    }

    decodeBool(_name: string): boolean
    {
        return Boolean(this.array.consumeByte());
    }

    decodeInt(_name: string): number
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

    decodeFloat32(_name: string): number
    {
        return this.array.consumeFloat32();
    }

    decodeFloat64(_name: string): number
    {
        return this.array.consumeFloat64();
    }

    decodeString(_name: string): string
    {
        const length: number = this.decodeInt(null);
        const utf8: Uint8Array = this.array.consumeUint8Array(length);
        return new TextDecoder().decode(utf8);
    }

    decodeBinary(_name: string): Uint8Array
    {
        const size: number = this.decodeInt(null);
        return this.array.consumeUint8Array(size);
    }

    close(): void
    {
    }

    peek(): number
    {
        const offset: number = this.array.mark();
                           this.openArray(null);
        const id: number = this.decodeInt(null);
        this.array.restore(offset);
        return id;
    }

    unwrap(type: Type)
    {
        const size: number = this.openArray(null);
        const id: number = this.decodeInt(null);
        type.decode(this);

        console.assert(2 == size);
        console.assert(type.__id == id);

        this.close();
    }

    //
    // Implementation
    //
    array: ConsumerUint8Array;
};

} // namespace hlib_codec

