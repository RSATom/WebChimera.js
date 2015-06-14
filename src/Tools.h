#pragma once

template<size_t ... Sequence>
struct StaticSequence
{
};

template<size_t Current, size_t ... Sequence>
struct StaticSequenceGenerator;

template<size_t ... Sequence>
struct StaticSequenceGenerator<0, Sequence ...>
{
    typedef StaticSequence<Sequence ...> SequenceType;
};

template<size_t Current, size_t ... Sequence>
struct StaticSequenceGenerator
{
    typedef typename StaticSequenceGenerator<Current - 1, Current - 1, Sequence ...>::SequenceType SequenceType;
};

template<size_t Length>
struct MakeStaticSequence
{
    typedef typename StaticSequenceGenerator<Length>::SequenceType SequenceType;
};
