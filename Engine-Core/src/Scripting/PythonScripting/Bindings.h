#pragma once

#include "ECS/Entity.h"
#include "PythonBase.h"

#include <string>


namespace Bindings {
#pragma region Entity Bindings
	static void CleanUpEntity(PyObject* capsule)
	{
		Ecs::Entity* obj = static_cast<Ecs::Entity*>(PyCapsule_GetPointer(capsule, "Entity"));
		if (obj)
			delete obj;
	}

	static PyObject* PyCreateEntity(PyObject* self, PyObject* args)
	{
		const char* val;
		if (!PyArg_ParseTuple(args, "s", &val))
			return nullptr;


		Ecs::Entity* obj = new Ecs::Entity(val);
		return PyCapsule_New(obj, "Entity", CleanUpEntity);
	}

	static PyObject* PyEntityGetName(PyObject* self, PyObject* args)
	{
		PyObject* capsule;
		if (!PyArg_ParseTuple(args, "O", &capsule))
			return nullptr;


		Ecs::Entity* obj = static_cast<Ecs::Entity*>(PyCapsule_GetPointer(capsule, "Entity"));
		if (!obj)
			return nullptr;

		const char* name = obj->GetName().data();

		PyObject* pyName = PyUnicode_FromString(name);
		return pyName;
	}

	static PyObject* PyEntityGetID(PyObject* self, PyObject* args)
	{
		PyObject* capsule;
		if (!PyArg_ParseTuple(args, "O", &capsule))
			return nullptr;

		Ecs::Entity* obj = static_cast<Ecs::Entity*>(PyCapsule_GetPointer(capsule, "Entity"));
		if (!obj)
			return nullptr;

		uint64_t ID = static_cast<uint64_t>(obj->GetID());
		return PyLong_FromUnsignedLongLong(ID);
	}
#pragma endregion Entity Bindings

#pragma region Logging
	static PyObject* LogMsg(PyObject* self, PyObject* args)
	{
		const char* val;
		if (!PyArg_ParseTuple(args, "s", &val))
			return nullptr;


		TGE_LOG_INFO(val);
		return PyLong_FromUnsignedLongLong(1);
	}

	static PyObject* LogWarn(PyObject* self, PyObject* args)
	{
		const char* val;
		if (!PyArg_ParseTuple(args, "s", &val))
			return nullptr;


		TGE_LOG_WARN(val);
		return PyLong_FromUnsignedLongLong(1);
	}

	static PyObject* LogError(PyObject* self, PyObject* args)
	{
		const char* val;
		if (!PyArg_ParseTuple(args, "s", &val))
			return nullptr;


		TGE_CLIENT_ERROR(std::string(val));
		return PyLong_FromUnsignedLongLong(1);
	}


#pragma endregion Logging

	static PyMethodDef TooGoodEngineMethods[] =
	{
		{"PyCreateEntity",     PyCreateEntity,		METH_VARARGS, "Create instance of an entity."},
		{"PyEntityGetName",    PyEntityGetName,		METH_VARARGS, "Get name from entity instance."},
		{"PyEntityGetID",	   PyEntityGetID,		METH_VARARGS, "Get an ID from an entity instance"},

		{"Log",						LogMsg,			METH_VARARGS, "Logs message"},
		{"LogWarn",					LogWarn,		METH_VARARGS, "Logs warning"},
		{"LogError",			    LogError,		METH_VARARGS, "Logs error"},

		{NULL, NULL, 0, NULL}
	};

	static struct PyModuleDef TooGoodEngineModule
	{
			PyModuleDef_HEAD_INIT,
			"TooGoodEngine",
			nullptr,
			-1,
			TooGoodEngineMethods
	};
}