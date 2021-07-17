import { makeDecorator } from './badrequire'

export function isExported(): boolean {
    return !!(makeDecorator !== undefined);
}
