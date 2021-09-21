package metacall

import (
	"sync/atomic"
	"unsafe"
)

type node struct {
	value interface{}
	next  unsafe.Pointer
}

type queue struct {
	head unsafe.Pointer
	tail unsafe.Pointer
}

func load(p *unsafe.Pointer) *node {
	return (*node)(atomic.LoadPointer(p))
}

func store(p *unsafe.Pointer, n *node) {
	atomic.StorePointer(p, unsafe.Pointer(n))
}

func cas(p *unsafe.Pointer, old, new *node) bool {
	return atomic.CompareAndSwapPointer(
		p, unsafe.Pointer(old), unsafe.Pointer(new))
}

func newQueue() (q *queue) {
	n := unsafe.Pointer(&node{})
	q = &queue{head: n, tail: n}

	return q
}

func (q *queue) enqueue(v interface{}) {
	n := &node{value: v}
	for {
		last := load(&q.tail)
		next := load(&last.next)
		if last == load(&q.tail) {
			if next == nil {
				if cas(&last.next, next, n) {
					cas(&q.tail, last, n)
					return
				}
			} else {
				cas(&q.tail, last, next)
			}
		}
	}
}

func (q *queue) dequeue() (interface{}, bool) {
	first := load(&q.head)
	last := load(&q.tail)
	next := load(&first.next)
	if first == load(&q.head) {
		if first == last {
			if next == nil {
				return nil, false
			}
			cas(&q.tail, last, next)
		} else {
			v := next.value
			if cas(&q.head, first, next) {
				return v, true
			}
		}
	}

	return nil, false
}

func (q *queue) iterator() <-chan interface{} {
	c := make(chan interface{})
	go func(q *queue, c chan<- interface{}) {
		for {
			v, ok := q.dequeue()
			if !ok {
				break
			}

			c <- v
		}
		close(c)
	}(q, c)
	return c
}

type Queue struct {
	impl     *queue
	watchers *queue
	quit     chan int
}

func NewQueue() *Queue {
	return &Queue{
		impl:     newQueue(),
		watchers: newQueue(),
		quit:     make(chan int),
	}
}

func (q *Queue) Enqueue(v interface{}) {
	q.impl.enqueue(v)

	for notify := range q.watchers.iterator() {
		c := notify.(chan int)

		go func() {
			c <- 1
		}()
	}
}

func (q *Queue) Dequeue() (interface{}, bool) {
	return q.impl.dequeue()
}

func (q *Queue) Iterator() <-chan interface{} {
	c := make(chan interface{})
	go func(q *Queue, c chan<- interface{}) {
		for {
			v, ok := q.Dequeue()

			if !ok {
				notify := make(chan int)
				q.watchers.enqueue(notify)

				select {
				case <-notify:
					continue
				case <-q.quit:
					go func() {
						<-notify
					}()
					goto end
				}

			} else {
				c <- v
			}
		}

	end:
		close(c)
	}(q, c)
	return c
}

func (q *Queue) Close() {
	go func() {
		q.quit <- 1
	}()
}
