import React from 'react';
import { renderToString } from 'react-dom/server';

export function hello(text: string): string {
	return renderToString(<h1>{`Hello ${text}`}</h1>);
}
