#pragma once
#include <iostream>
using namespace std;
template <typename T>
class QNode
{
public:
	T data;
	QNode* next;
public:
	QNode() {}
	QNode(T val) : data(val), next(nullptr) {}
};

template <typename T>
class Queue
{
	QNode<T>* front;
	QNode<T>* rear;
public:
	Queue() : front(nullptr), rear(nullptr) {}

	bool is_empty()
	{
		return front == nullptr; //no elements = no front
	}

	void print()
	{
		QNode<T>* current = front;
		while (current)
		{
			cout << current->data << " ";
			current = current->next;
		}
		cout << endl;
	}

	void enqueue(T item)
	{
		QNode<T>* temp = new QNode<T>(item);
		if (is_empty()) //front and rear need to be set
		{
			rear = temp;
			front = rear;
		}
		else
		{
			rear->next = temp;
			rear = temp; //move rear forward
		}
	}

	T dequeue() //return dequeued value
	{
		if (is_empty()) //dont dequeue an empty list
			return T();
		//dequeue from the front
		QNode<T>* toDelete = front;
		front = front->next; //move front forward
		T data = toDelete->data;
		delete toDelete;
		return data;
	}

	T Front() //return the data
	{
		if (is_empty())
			return T();
		return front->data;
	}

	QNode<T>* peek() //return the pointer
	{
		if (is_empty())
			return nullptr;
		return front;
	}

	void clear() //empty the queue
	{
		while (!is_empty())
			dequeue();
	}

	int size()
	{
		QNode<T>* current = front;
		int size = 0;
		while (current)
		{
			size++;
			current = current->next;
		}
		return size;
	}

	bool contains(const T& value)
	{
		QNode<T>* current = front;
		while (current)
		{
			if (current->data == value)
				return true;
			current = current->next;
		}
		return false;
	}
};

